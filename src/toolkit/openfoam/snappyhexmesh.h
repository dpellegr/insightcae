/*
 * This file is part of Insight CAE, a workbench for Computer-Aided Engineering 
 * Copyright (C) 2014  Hannes Kroeger <hannes@kroegeronline.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 */

#ifndef INSIGHT_SNAPPYHEXMESH_H
#define INSIGHT_SNAPPYHEXMESH_H

#include <string>
#include <vector>

#include "base/boost_include.h"

#include "openfoam/caseelements/openfoamcaseelement.h"
#include "base/progressdisplayer.h"

namespace insight {
  
    
    
    
class ExternalGeometryFile
{
public:
#include "snappyhexmesh__ExternalGeometryFile__Parameters.h"
/*
PARAMETERSET>>> ExternalGeometryFile Parameters

fileName = path "" "Path to geometry file (STL format)" *necessary
scale = vector (1 1 1) "Geometry scaling factor for each spatial direction"
translate = vector (0 0 0) "Translation vector"
rollPitchYaw = vector (0 0 0) "Euler angles around X, Y and Z axis respectively"

<<<PARAMETERSET
*/

protected:
  Parameters p_;

public:
  ExternalGeometryFile( const ParameterSet& ps = Parameters::makeDefault() );
  
  std::string fileName() const;
  
  virtual void putIntoConstantTrisurface(
      const OpenFOAMCase& ofc,
      const boost::filesystem::path& location
      ) const;
};




namespace snappyHexMeshFeats
{

class Feature;

typedef std::shared_ptr<Feature> FeaturePtr;
    



class Feature
{
public:
  declareType ( "Feature" );

  declareDynamicClass ( Feature );

  virtual void addIntoDictionary ( OFDictData::dict& sHMDict ) const =0;
  virtual void modifyFiles (
      const OpenFOAMCase& ofc,
      const boost::filesystem::path& location ) const;
};




class Geometry
: public Feature,
  public ExternalGeometryFile
{
public:
#include "snappyhexmesh__Geometry__Parameters.h"
/*
PARAMETERSET>>> Geometry Parameters
inherits insight::ExternalGeometryFile::Parameters

name = string "unnamed" "Name of the geometry feature"  *necessary
minLevel = int 0 "Minimum refinement level"
maxLevel = int 4 "Maximum refinement level"
nLayers = int 2 "Number of prism layers"
zoneName = string "" "Zone name"

regionRefinements = array [ set {
 regionname = string "" "Name of geometry region" *necessary
 minLevel = int 0 "Minimum refinement level"
 maxLevel = int 0 "Maximum refinement level"
} ]*0 "Refinement regions"

<<<PARAMETERSET
*/

protected:
  Parameters p_;

public:
  declareType("Geometry");

  Geometry(const ParameterSet& ps = Parameters::makeDefault() );
  ParameterSet getParameters() const override { return p_; }
  inline const Parameters& parameters() const { return p_; }
  
  void addIntoDictionary(OFDictData::dict& sHMDict) const override;
  void modifyFiles(const OpenFOAMCase& ofc,
                   const boost::filesystem::path& location) const override;
};




class PatchLayers
: public Feature
{
public:
#include "snappyhexmesh__PatchLayers__Parameters.h"
/*
PARAMETERSET>>> PatchLayers Parameters

name = string "" "Name of the patch"  *necessary
nLayers = int 2 "Number of layers"

<<<PARAMETERSET
*/

protected:
  Parameters p_;

public:
  declareType("PatchLayers");

  PatchLayers(const ParameterSet& ps = Parameters::makeDefault() );
  ParameterSet getParameters() const override { return p_; }
  inline const Parameters& parameters() const { return p_; }

  void addIntoDictionary(OFDictData::dict& sHMDict) const override;
};




class ExplicitFeatureCurve
: public Feature
{
public:
#include "snappyhexmesh__ExplicitFeatureCurve__Parameters.h"
/*
PARAMETERSET>>> ExplicitFeatureCurve Parameters

fileName = path "" "Filename of the feature curve"  *necessary
scale = vector (1 1 1) "Geometry scaling factor for each spatial direction"
translate = vector (0 0 0) "Translation vector"
rollPitchYaw = vector (0 0 0) "Euler angles around X, Y and Z axis respectively"
level = int 4 "Refinement level at curve"
distance = double 0 "Refinement distance"

<<<PARAMETERSET
*/

protected:
  Parameters p_;

public:
  declareType("ExplicitFeatureCurve");

  ExplicitFeatureCurve(const ParameterSet& ps = Parameters::makeDefault() );
  ParameterSet getParameters() const override { return p_; }
  inline const Parameters& parameters() const { return p_; }

  void addIntoDictionary(OFDictData::dict& sHMDict) const override;
  void modifyFiles(const OpenFOAMCase& ofc,
                   const boost::filesystem::path& location) const override;
};




class RefinementRegion
  : public Feature
{
public:
#include "snappyhexmesh__RefinementRegion__Parameters.h"
/*
PARAMETERSET>>> RefinementRegion Parameters

name = string "" "Region name"  *necessary
dist = double 1e15 "Maximum distance for refinement"
mode = selection ( inside outside distance ) inside "Refinement mode"
level = int 1 "Refinement level"

<<<PARAMETERSET
*/

protected:
  Parameters p_;

public:
  declareType ( "RefinementRegion" );

  RefinementRegion ( const ParameterSet& ps = Parameters::makeDefault() );

  inline const Parameters& parameters() const { return p_; }

  /**
   * create entry into geometry subdict.
   * supply handle to title, since it is not always equal to name and there might be need to change it
   */
  virtual bool setGeometrySubdict ( OFDictData::dict& d, std::string& entryTitle ) const =0;
  void addIntoDictionary ( OFDictData::dict& sHMDict ) const override;
};




class RefinementBox
  : public RefinementRegion
{
public:
#include "snappyhexmesh__RefinementBox__Parameters.h"
/*
PARAMETERSET>>> RefinementBox Parameters
inherits insight::snappyHexMeshFeats::RefinementRegion::Parameters

min = vector (0 0 0) "Minimum corner of refinement box" *necessary
max = vector (1 1 1) "Maximum corner of refinement box" *necessary

<<<PARAMETERSET
*/

protected:
  Parameters p_;

public:
  declareType ( "RefinementBox" );

  RefinementBox ( const ParameterSet& ps = Parameters::makeDefault() );
  ParameterSet getParameters() const override
  {
    return p_;
  }
  inline const Parameters& parameters() const { return p_; }

  bool setGeometrySubdict ( OFDictData::dict& d, std::string& entryTitle ) const override;
};




class RefinementCylinder
  : public RefinementRegion
{
public:
#include "snappyhexmesh__RefinementCylinder__Parameters.h"
/*
PARAMETERSET>>> RefinementCylinder Parameters
inherits insight::snappyHexMeshFeats::RefinementRegion::Parameters

point1 = vector (0 0 0) "Base point of the refinement cylinder" *necessary
point2 = vector (1 0 0) "Tip point of the refinement cylinder" *necessary
radius = double 1 "Radius of the refinement region" *necessary

<<<PARAMETERSET
*/

protected:
  Parameters p_;

public:
  declareType ( "RefinementCylinder" );

  RefinementCylinder ( const ParameterSet& ps = Parameters::makeDefault() );
  ParameterSet getParameters() const override
  {
    return p_;
  }
  inline const Parameters& parameters() const { return p_; }

  bool setGeometrySubdict ( OFDictData::dict& d, std::string& entryTitle ) const override;
};



class RefinementSphere
  : public RefinementRegion
{
public:
#include "snappyhexmesh__RefinementSphere__Parameters.h"
/*
PARAMETERSET>>> RefinementSphere Parameters
inherits insight::snappyHexMeshFeats::RefinementRegion::Parameters

center = vector (0 0 0) "Center of the refinement sphere" *necessary
radius = double 1 "Radius of the refinement region" *necessary

<<<PARAMETERSET
*/

protected:
  Parameters p_;

public:
  declareType ( "RefinementSphere" );

  RefinementSphere ( const ParameterSet& ps = Parameters::makeDefault() );
  ParameterSet getParameters() const override
  {
    return p_;
  }
  inline const Parameters& parameters() const { return p_; }

  bool setGeometrySubdict ( OFDictData::dict& d, std::string& entryTitle ) const override;
};




class RefinementGeometry
: public RefinementRegion
{
public:
#include "snappyhexmesh__RefinementGeometry__Parameters.h"
/*
PARAMETERSET>>> RefinementGeometry Parameters
inherits insight::snappyHexMeshFeats::RefinementRegion::Parameters

geometry = includedset "insight::ExternalGeometryFile::Parameters" "Geometry file (STL format)"

<<<PARAMETERSET
*/
//fileName = path "" "Path to geometry file (STL format)"

protected:
  Parameters p_;
  ExternalGeometryFile geometryfile_;

public:
  declareType("RefinementGeometry");

  RefinementGeometry( const ParameterSet& ps = Parameters::makeDefault() );
  ParameterSet getParameters() const override { return p_; }
  inline const Parameters& parameters() const { return p_; }

  bool setGeometrySubdict(OFDictData::dict& d, std::string& entryTitle) const override;
  //   virtual void addIntoDictionary(OFDictData::dict& sHMDict) const;
  void modifyFiles(const OpenFOAMCase& ofc,
                   const boost::filesystem::path& location) const override;

};




class NearSurfaceRefinement
: public RefinementRegion
{
public:
  declareType("NearSurfaceRefinement");

  NearSurfaceRefinement( const ParameterSet& ps = Parameters::makeDefault() );
  ParameterSet getParameters() const override { return p_; }
  inline const Parameters& parameters() const { return p_; }

  bool setGeometrySubdict(OFDictData::dict& d, std::string& entryTitle) const override;
};




class NearTemplatePatchRefinement
  : public RefinementRegion
{
public:
#include "snappyhexmesh__NearTemplatePatchRefinement__Parameters.h"
/*
PARAMETERSET>>> NearTemplatePatchRefinement Parameters
inherits insight::snappyHexMeshFeats::RefinementRegion::Parameters

fileName = path "" "Path to geometry file (STL format)" *necessary

<<<PARAMETERSET
*/

protected:
  Parameters p_;

public:
  declareType ( "NearTemplatePatchRefinement" );

  NearTemplatePatchRefinement ( const ParameterSet& ps = Parameters::makeDefault() );
  ParameterSet getParameters() const override { return p_; }
  inline const Parameters& parameters() const { return p_; }

  void modifyFiles ( const OpenFOAMCase& ofc,
                     const boost::filesystem::path& location ) const override;
  bool setGeometrySubdict ( OFDictData::dict& d, std::string& entryTitle ) const override;
};




}





class snappyHexMeshConfiguration
    : public OpenFOAMCaseElement
{
public:
#include "snappyhexmesh__snappyHexMeshConfiguration__Parameters.h"
/*
PARAMETERSET>>> snappyHexMeshConfiguration Parameters

doCastellatedMesh = bool true "Enable castellated meshing step"
doSnap = bool true "Enable snapping step"
doAddLayers = bool true "Enable layer addition step"
tlayer= double 0.5 "Layer thickness value"
erlayer = double 1.3 "Expansion ratio of layers"
relativeSizes = bool true "Whether tlayer specifies relative thickness (absolute thickness if set to false)"
nLayerIter= int 10 "Maximum number of layer iterations"
stopOnBadPrismLayer = bool false "Whether to stop of too few layers get extruded"

qualityCtrls = selection ( standard relaxed disabled ) relaxed "Select quality requirements"

PiM = array [ vector (0 0 0) "point inside mesh" ]*0 "One or more points inside meshing domain"
PiMZoneNames = array [ string "zone%d"
 "name of zone which is created from the PiM with the same index as this name.
 May contain %d as a placeholder of the zone index starting from one.
 If this list is shorter than the list of PiMs, the default 'zone%d' will be used for the remainder." ]*0
 "Names for the cell zones which are created by the according points in mesh."

features = array [
 dynamicclassconfig "insight::snappyHexMeshFeats::Feature" default "Geometry" "SnappyHexMesh feature"
]*0 "Mesh generation features" *necessary

doExplicitFeatureSnap = bool false "Disable/Enable snapping of explicit features (eMesh)"
doImplicitFeatureSnap = bool true "Disable/Enable snapping to implicit features"
nSmoothPatch = int 3 "Number of patch smoothing operations"

allowFreeStandingZoneFaces = bool true "allowFreeStandingZoneFaces"
    
<<<PARAMETERSET
*/

protected:
  Parameters p_;

public:
  declareType ( "snappyHexMeshConfiguration" );

  snappyHexMeshConfiguration ( OpenFOAMCase& c, const ParameterSet& ps = Parameters::makeDefault() );
  void addIntoDictionaries ( OFdicts& dictionaries ) const override;
  void modifyCaseOnDisk ( const OpenFOAMCase& cm, const boost::filesystem::path& location ) const override;

  static std::string category() { return "Meshing"; }

};




void setStdCastellatedCtrls(OFDictData::dict& castellatedCtrls);
void setStdSnapCtrls(OFDictData::dict& snapCtrls);
void setStdLayerCtrls(OFDictData::dict& layerCtrls);
void setStdQualityCtrls(OFDictData::dict& qualityCtrls);
void setRelaxedQualityCtrls(OFDictData::dict& qualityCtrls);
void setDisabledQualityCtrls(OFDictData::dict& qualityCtrls);
void setNoQualityCtrls(OFDictData::dict& qualityCtrls);




double computeFinalLayerThickness(double totalLayerThickness, double expRatio, int nlayer);




void reconstructParMesh
(
  const OpenFOAMCase& ofc,
  const boost::filesystem::path& location
);




void snappyHexMesh
(
  const OpenFOAMCase& ofc, 
  const boost::filesystem::path& location, 
//   const OFDictData::list& PiM,
//   const std::vector<snappyHexMeshFeats::FeaturePtr>& ops,
//   snappyHexMeshOpts::Parameters const& p = snappyHexMeshOpts::Parameters(),
  const ParameterSet &ps = snappyHexMeshConfiguration::Parameters::makeDefault(),
  bool overwrite=true,
  bool isalreadydecomposed=false,
  bool keepdecomposedafterfinish=false,
  ProgressDisplayer* progress=nullptr,
  std::function<void(OFDictData::dict&)> sHMDictModifier = std::function<void(OFDictData::dict&)>()
);



}

#endif // INSIGHT_SNAPPYHEXMESH_H
