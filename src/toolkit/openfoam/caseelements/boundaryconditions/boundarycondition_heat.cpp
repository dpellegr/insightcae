
#include "openfoam/caseelements/boundaryconditions/boundarycondition_heat.h"

using namespace std;
using namespace boost;
using namespace boost::filesystem;
using namespace boost::assign;
using namespace boost::fusion;

namespace insight
{
namespace HeatBC
{

defineType(HeatBC);
defineDynamicClass(HeatBC);

HeatBC::~HeatBC()
{}

void HeatBC::addOptionsToBoundaryDict(OFDictData::dict &) const
{}


void HeatBC::addIntoDictionaries(OFdicts&) const
{}




defineType(AdiabaticBC);
addToFactoryTable(HeatBC, AdiabaticBC);
addToStaticFunctionTable(HeatBC, AdiabaticBC, defaultParameters);

AdiabaticBC::AdiabaticBC(const ParameterSet&)
{}

bool AdiabaticBC::addIntoFieldDictionary(const string& fieldname, const FieldInfo& fieldinfo, OFDictData::dict& BC, OFdicts&) const
{
    if
    (
      (fieldname=="T")
      &&
      (get<0>(fieldinfo)==scalarField)
    )
    {
      BC["type"]="zeroGradient";
      return true;
    }
    else
      return false;
}





defineType(FixedTemperatureBC);
addToFactoryTable(HeatBC, FixedTemperatureBC);
addToStaticFunctionTable(HeatBC, FixedTemperatureBC, defaultParameters);

FixedTemperatureBC::FixedTemperatureBC(const ParameterSet& ps)
: p_(ps)
{}

bool FixedTemperatureBC::addIntoFieldDictionary(const string& fieldname, const FieldInfo& fieldinfo, OFDictData::dict& BC, OFdicts& dictionaries) const
{
    if
    (
      (fieldname=="T")
      &&
      (get<0>(fieldinfo)==scalarField)
    )
    {
      FieldData(p_.T).setDirichletBC(BC, dictionaries);
      return true;
    }
    else
      return false;
}




defineType(TemperatureGradientBC);
addToFactoryTable(HeatBC, TemperatureGradientBC);
addToStaticFunctionTable(HeatBC, TemperatureGradientBC, defaultParameters);

TemperatureGradientBC::TemperatureGradientBC(const ParameterSet& ps)
: p_(ps)
{}

bool TemperatureGradientBC::addIntoFieldDictionary(const string& fieldname, const FieldInfo& fieldinfo, OFDictData::dict& BC, OFdicts& dictionaries) const
{
    if
    (
      (fieldname=="T")
      &&
      (get<0>(fieldinfo)==scalarField)
    )
    {
      BC["type"]="uniformFixedGradient";
      std::ostringstream tableData;
      if (const auto* cd = boost::get<Parameters::gradT_constant_type>(&p_.gradT))
      {
        tableData << cd->gradT;
      }
      else if (const auto* ud = boost::get<Parameters::gradT_unsteady_type>(&p_.gradT))
      {
        tableData << "table (";
        for (const auto& d: ud->gradT_vs_t)
        {
          tableData << " (" << d.t<<" "<<d.gradT<<")";
        }
        tableData << " )";
      }
      BC["uniformGradient"]=tableData.str();
      return true;
    }
    else
      return false;
}



defineType(ExternalWallBC);
addToFactoryTable(HeatBC, ExternalWallBC);
addToStaticFunctionTable(HeatBC, ExternalWallBC, defaultParameters);

ExternalWallBC::ExternalWallBC(const ParameterSet& ps)
: p_(ps)
{}

bool ExternalWallBC::addIntoFieldDictionary(const string& fieldname, const FieldInfo& fieldinfo, OFDictData::dict& BC, OFdicts&) const
{
    if
    (
      (fieldname=="T")
      &&
      (get<0>(fieldinfo)==scalarField)
    )
    {
        BC["type"]="externalWallHeatFluxTemperature";

        if (const auto* ft = boost::get<Parameters::kappaSource_fluidThermo_type>(&p_.kappaSource))
        {
          BC["kappaMethod"]="fluidThermo";
          BC["kappa"]="none";
        }
        else if (const auto* lu = boost::get<Parameters::kappaSource_lookup_type>(&p_.kappaSource))
        {
          BC["kappaMethod"]="lookup";
          BC["kappa"]=lu->kappaFieldName;
        }

        BC["Qr"]="none";
        BC["relaxation"]=1.0;

        if (const auto* const_p = boost::get<Parameters::heatflux_fixedPower_type>(&p_.heatflux))
          {
            BC["mode"]="power";
            BC["Q"]=boost::str(boost::format("%g")%const_p->Q );
          }
        else if (const auto* const_q = boost::get<Parameters::heatflux_fixedHeatFlux_type>(&p_.heatflux))
          {
            BC["mode"]="flux";
            BC["q"]=boost::str(boost::format("uniform %g")%const_q->q );
          }
        else if (const auto* conv_q = boost::get<Parameters::heatflux_fixedHeatTransferCoeff_type>(&p_.heatflux))
          {
            BC["mode"]="coefficient";
            BC["Ta"]=boost::str(boost::format("uniform %g") % conv_q->Ta );
            BC["h"]=boost::str(boost::format("uniform %g") % conv_q->h );
          }

        OFDictData::list tL, kL;
        for (const Parameters::wallLayers_default_type& layer: p_.wallLayers)
        {
          tL.push_back(layer.thickness);
          kL.push_back(layer.kappa);
        }
        BC["thicknessLayers"]=tL;
        BC["kappaLayers"]=kL;

        BC["value"]=boost::str(boost::format("uniform %g") % 300.0 );
      return true;
    }
    else
      return false;
}





defineType(CHTCoupledWall);
addToFactoryTable(HeatBC, CHTCoupledWall);
addToStaticFunctionTable(HeatBC, CHTCoupledWall, defaultParameters);

CHTCoupledWall::CHTCoupledWall(const ParameterSet& ps)
  : p_(ps)
{}

void CHTCoupledWall::addOptionsToBoundaryDict(OFDictData::dict &BCdict) const
{
  HeatBC::addOptionsToBoundaryDict(BCdict);

  BCdict["type"]="mappedWall";

  std::string method;
  switch (p_.method)
  {
    case Parameters::nearestPatchFace: method="nearestPatchFace"; break;
    case Parameters::nearestPatchFaceAMI: method="nearestPatchFaceAMI"; break;
  }
  BCdict["sampleMode"]=method;

  BCdict["sampleRegion"]=p_.sampleRegion;
  BCdict["samplePatch"]=p_.samplePatch;

  if (const auto *uofs = boost::get<Parameters::offset_uniform_type>(&p_.offset))
  {
    BCdict["offset"]=OFDictData::vector3(uofs->distance);
  }
}

bool CHTCoupledWall::addIntoFieldDictionary(const string& fieldname, const FieldInfo& fieldinfo, OFDictData::dict& BC, OFdicts&) const
{
    if
    (
      (fieldname=="T")
      &&
      (get<0>(fieldinfo)==scalarField)
    )
    {
        BC["type"]="compressible::turbulentTemperatureCoupledBaffleMixed";
        BC["Tnbr"]=p_.Tnbr;
        BC["thicknessLayers"]=OFDictData::list();
        BC["kappaLayers"]=OFDictData::list();
        BC["kappaMethod"]="fluidThermo";
        BC["kappa"]="none";
        BC["alphaAni"]="none";


        BC["value"]=boost::str(boost::format("uniform %g") % 300.0 );
      return true;
    }
    else
      return false;
}


}
}
