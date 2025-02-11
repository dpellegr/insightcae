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


#ifndef INSIGHT_OPENFOAMANALYSIS_H
#define INSIGHT_OPENFOAMANALYSIS_H

#include "base/analysis.h"

#include "openfoam/caseelements/turbulencemodel.h"
#include "base/progressdisplayer/textprogressdisplayer.h"
#include "base/progressdisplayer/convergenceanalysisdisplayer.h"



namespace insight {



class OpenFOAMCase;
class ConvergenceAnalysisDisplayer;


class OpenFOAMAnalysis
: public Analysis
{
public:
  
#include "openfoamanalysis__OpenFOAMAnalysis__Parameters.h"
/*
PARAMETERSET>>> OpenFOAMAnalysis Parameters

run = set 
{	
 machine 	= 	string 	"" 	"Machine or queue, where the external commands are executed on. Defaults to 'localhost', if left empty." *hidden
 OFEname 	= 	string 	"OFesi1806" "Identifier of the OpenFOAM installation, that shall be used"
 np 		= 	int 	1 	"Number of processors for parallel run (less or equal 1 means serial execution)" *necessary
 mapFrom 	= 	path 	"" 	"Map solution from specified case, if not empty. potentialinit is skipped if specified."
 potentialinit 	= 	bool 	false 	"Whether to initialize the flow field by potentialFoam when no mapping is done" *hidden
 evaluateonly	= 	bool 	false 	"Whether to skip solver run and do only the evaluation"
} "Execution parameters"

mesh = set
{
 linkmesh 	= 	path 	"" 	"If not empty, the mesh will not be generated, but a symbolic link to the polyMesh folder of the specified OpenFOAM case will be created." *hidden
} "Properties of the computational mesh"

fluid = set
{
 turbulenceModel = dynamicclassparameters "insight::turbulenceModel" default "kOmegaSST" "Turbulence model"
} "Parameters of the fluid"

eval = set
{
 reportdicts 	= 	bool 	true 	"Include dictionaries into report" *hidden
 skipmeshquality 	= 	bool 	false 	"Check to exclude mesh check during evaluation" *hidden
} "Parameters for evaluation after solver run"

<<<PARAMETERSET
*/

protected:
    ResultSetPtr derivedInputData_;
    
    std::vector<std::shared_ptr<ConvergenceAnalysisDisplayer> > convergenceAnalysis_;

public:
    OpenFOAMAnalysis
    (
        const std::string& name,
        const std::string& description,
        const ParameterSet& ps,
        const boost::filesystem::path& exepath
    );

    boost::filesystem::path setupExecutionEnvironment() override;
    
    void initializeDerivedInputDataSection();
    virtual void reportIntermediateParameter(const std::string& name, double value, const std::string& description="", const std::string& unit="");
    
    virtual void calcDerivedInputData(ProgressDisplayer& progress);
    virtual void createMesh(OpenFOAMCase& cm, ProgressDisplayer& progress) =0;
    virtual void createCase(OpenFOAMCase& cm, ProgressDisplayer& progress) =0;
    
    virtual void createDictsInMemory(OpenFOAMCase& cm, std::shared_ptr<OFdicts>& dicts);
    
    virtual void writeDictsToDisk(OpenFOAMCase& cm, std::shared_ptr<OFdicts>& dicts);
    /**
     * Customize dictionaries before they get written to disk
     */
    virtual void applyCustomOptions(OpenFOAMCase& cm, std::shared_ptr<OFdicts>& dicts);
    
    
    /**
     * Do modifications to the case when it has been created on disk
     */
    virtual void applyCustomPreprocessing(OpenFOAMCase& cm, ProgressDisplayer& progress);
    
    virtual void mapFromOther(OpenFOAMCase& cm, ProgressDisplayer& progress, const boost::filesystem::path& mapFromPath, bool is_parallel);
    
    virtual void installConvergenceAnalysis(std::shared_ptr<ConvergenceAnalysisDisplayer> cc);
    
    
    /**
     * @brief prepareCaseCreation
     * perform action prior to actual case setup,
     * e.g. running another solver to obtain corrections for settings
     * @param progress
     */
    virtual void prepareCaseCreation(ProgressDisplayer& progress);

    /**
     * integrate all steps before the actual run
     */
    virtual void createCaseOnDisk(OpenFOAMCase& cm, ProgressDisplayer& progress);
    
    virtual void initializeSolverRun(ProgressDisplayer& progress, OpenFOAMCase& cm);
    virtual void runSolver(ProgressDisplayer& displayer, OpenFOAMCase& cm);
    virtual void finalizeSolverRun(OpenFOAMCase& cm, ProgressDisplayer& progress);

    virtual ResultSetPtr evaluateResults(OpenFOAMCase& cm, ProgressDisplayer& progress);

    ResultSetPtr operator()(ProgressDisplayer& displayer = consoleProgressDisplayer ) override;
};


turbulenceModel* insertTurbulenceModel(OpenFOAMCase& cm, const OpenFOAMAnalysis::Parameters& params );
turbulenceModel* insertTurbulenceModel(OpenFOAMCase& cm, const SelectableSubsetParameter& ps );

}

#endif // INSIGHT_OPENFOAMANALYSIS_H
