#ifndef VTKRENDERING_H
#define VTKRENDERING_H

#include <vector>
#include <string>
#include <map>

#include "base/boost_include.h"
#include "base/linearalgebra.h"

#include "vtkSmartPointer.h"
#include "vtkPolyData.h"
#include "vtkLookupTable.h"
#include "vtkCamera.h"
#include "vtkScalarBarActor.h"
#include "vtkRenderWindow.h"
#include "vtkActor.h"
#include "vtkColorSeries.h"
#include "vtkProperty.h"
#include "vtkRenderer.h"
#include "vtkCellData.h"
#include "vtkPointData.h"
#include "vtkAlgorithm.h"
#include "vtkOpenFOAMReader.h"
#include "vtkUnstructuredGrid.h"
#include "vtkMultiBlockDataSet.h"
#include "vtkCompositeDataGeometryFilter.h"




namespace insight
{


vtkSmartPointer<vtkPolyData> createArrows(
    std::vector<std::pair<arma::mat, arma::mat> > from_to
    );


extern const std::vector<double> colorMapData_SD;


vtkSmartPointer<vtkLookupTable> createColorMap(
        const std::vector<double>& cb = colorMapData_SD,
        int nc=32
        );



typedef std::pair<double, double> MinMax;

typedef enum { Cell, Point } FieldSupport;

struct FieldSelection
   : public boost::fusion::tuple
    <
     std::string,
     FieldSupport,
     int
    >
{
  FieldSelection(
      std::string fieldName,
      FieldSupport fieldSupport,
      int component=-1
      );

  std::string fieldName() const;
};

typedef boost::variant<boost::blank,MinMax> RangeSelection;

struct FieldColor : public boost::fusion::tuple
    <
     FieldSelection,
     vtkSmartPointer<vtkLookupTable>,
     RangeSelection
    >
{
  FieldColor(FieldSelection fieldSelection,
             vtkSmartPointer<vtkLookupTable> lookupTable,
             RangeSelection rangeSelection);

  vtkSmartPointer<vtkLookupTable> lookupTable() const;
};

typedef boost::variant
    <
     arma::mat,
     FieldColor
    >
        ColorSpecification;



MinMax calcRange(
    FieldSelection fsel,
    const std::vector<vtkDataSet*> datasets,
    const std::vector<vtkAlgorithm*> algorithms
    );


class MultiBlockDataSetExtractor
{
  vtkMultiBlockDataSet* mbds_;
  std::map<vtkDataObject*,int> flatIndices_;


public:
  MultiBlockDataSetExtractor(vtkMultiBlockDataSet* mbds);

  std::set<int> flatIndices(const std::vector<std::string>& groupNamePatterns) const;

  static std::set<vtkDataObject*> findObjectsBelowGroup(const std::string& name_pattern, vtkDataObject* input);
};


class VTKOffscreenScene
{

public:


protected:
  vtkSmartPointer<vtkRenderer> renderer_;
  vtkSmartPointer<vtkRenderWindow> renderWindow_;

public:
  VTKOffscreenScene();
  ~VTKOffscreenScene();

  template<class Mapper, class Input>
  void addAlgo(
      Input input,
      ColorSpecification colorspec = ColorSpecification(insight::vec3(0,0,0))
      )
  {
    input->Update();
    addData<Mapper>(input->GetOutput(), colorspec);
  }

  template<class Mapper, class Input>
  void addData(
      Input input,
      ColorSpecification colorspec = ColorSpecification(insight::vec3(0,0,0))
      )
  {
    auto mapper = vtkSmartPointer<Mapper>::New();
    mapper->SetInputData(input);

    addProperties<Mapper>(mapper, colorspec, input);
  }

  void addActor2D(vtkSmartPointer<vtkActor2D> actor);

  template<class Mapper>
  void addProperties(
      vtkSmartPointer<Mapper>& mapper,
      ColorSpecification colorspec,
          vtkDataSet *ds
      )
  {
    auto actor = vtkSmartPointer<vtkActor>::New();

    if (const auto* c = boost::get<arma::mat>(&colorspec))
    {
      mapper->ScalarVisibilityOff();
      actor->GetProperty()->SetColor((*c)(0), (*c)(1), (*c)(2));
    }
    else if (const auto *c = boost::get<FieldColor>(&colorspec))
    {
      auto fsel=boost::fusion::get<0>(*c);
      auto fieldname = boost::fusion::get<0>(fsel);
      auto fs = boost::fusion::get<1>(fsel);
      auto cmpt = boost::fusion::get<2>(fsel);
      auto lut = boost::fusion::get<1>(*c);
      auto rsel = boost::fusion::get<2>(*c);

      mapper->SetLookupTable(lut);
      mapper->ScalarVisibilityOn();
      switch (fs)
      {
        case Point:
          mapper->SetScalarModeToUsePointFieldData();
          break;
        case Cell:
          mapper->SetScalarModeToUseCellFieldData();
          break;
      }

      if (cmpt<0)
      {
        lut->SetVectorModeToMagnitude();
      }
      else
      {
        lut->SetVectorModeToComponent();
        lut->SetVectorComponent(cmpt);
      }
      mapper->SelectColorArray(fieldname.c_str());
      MinMax mm;
      if (const auto *mima = boost::get<MinMax>(&rsel))
      {
          mm=*mima;
      }
      else
      {
          mm=calcRange(fsel, {ds}, {});
      }
      mapper->SetScalarRange(mm.first, mm.second);

    }

    actor->SetMapper(mapper);
    actor->GetProperty()->BackfaceCullingOff();

    renderer_->AddActor(actor);
  }

  vtkSmartPointer<vtkScalarBarActor> addColorBar(
      const std::string& title,
      vtkSmartPointer<vtkLookupTable> lut,
      double x=0.9, double y=0.1,
      bool horiz=false,
      double w=0.09, double len=0.8,
      double fontmult=3.
      );


  void exportX3D(const boost::filesystem::path& file);
  void exportImage(const boost::filesystem::path& pngfile);

  vtkCamera* activeCamera();

  void setParallelScale(
      boost::variant<
        double, // scale
        std::pair<double,double> // Lh, Lv
      > scaleOrSize
      );

  void fitAll(double mult=1.05);

  void clearScene();

  void removeActor(vtkActor* act);
  void removeActor2D(vtkActor2D* act);
};








class OpenFOAMCaseScene
  : public VTKOffscreenScene,
    public vtkSmartPointer<vtkOpenFOAMReader>
{
  vtkSmartPointer<vtkOpenFOAMReader> ofcase_;
  std::map<std::string,int> patches_;
  vtkSmartPointer<vtkDoubleArray> times_;

public:
  OpenFOAMCaseScene(const boost::filesystem::path& casepath);

  vtkDoubleArray* times() const;
  void setTimeValue(double t);
  void setTimeIndex(vtkIdType timeId);

  vtkSmartPointer<vtkOpenFOAMReader> ofcase() const;
  vtkUnstructuredGrid* internalMesh() const;

  std::vector<std::string> matchingPatchNames(const std::string& patchNamePattern) const;

  vtkPolyData* patch(const std::string& name) const;
  vtkSmartPointer<vtkPolyData> patches(const std::string& namePattern) const;

  vtkSmartPointer<vtkCompositeDataGeometryFilter> extractBlock(int blockIdx) const;
  vtkSmartPointer<vtkCompositeDataGeometryFilter> extractBlock(const std::string& name) const;
  vtkSmartPointer<vtkCompositeDataGeometryFilter> extractBlock(const std::vector<int>& blockIdxs) const;
};




}

#endif // VTKRENDERING_H
