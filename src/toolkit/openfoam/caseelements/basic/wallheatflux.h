#ifndef INSIGHT_WALLHEATFLUX_H
#define INSIGHT_WALLHEATFLUX_H

#include "openfoam/caseelements/openfoamcaseelement.h"

namespace insight {

class wallHeatFlux
    : public OpenFOAMCaseElement
{

public:
#include "wallheatflux__wallHeatFlux__Parameters.h"
/*
PARAMETERSET>>> wallHeatFlux Parameters

name = string "wallHeatFlux" "Arbitrary label of the functionObject."
patches = array [ string "" "Name of a patch over which the heat flux shall be integrated. Regular expressions are recognized, if it is put in quotes." ] *1 "Patches"
region = string "region0" "Name of the region, for which the heat flux shall be determined."
qr = selectablesubset {{
 none set { }
 field set {
  fieldName = string "qr" "Name of the radiation heat flux field."
 }
}} none "Treatment of radiation heat flux"

<<<PARAMETERSET
*/

protected:
    Parameters p_;

public:
    declareType ( "wallHeatFlux" );
    wallHeatFlux ( OpenFOAMCase& c, const ParameterSet& ps = Parameters::makeDefault() );
    void addIntoDictionaries ( OFdicts& dictionaries ) const override;

    inline const std::string& name() const { return p_.name; }

    static std::map<std::string,arma::mat> readWallHeatFlux(
        const OpenFOAMCase& c,
        const boost::filesystem::path& location,
        const std::string& regionName,
        const std::string& foName );

    static std::string category() { return "Postprocessing"; }
};

} // namespace insight

#endif // INSIGHT_WALLHEATFLUX_H
