#include "openfoamcasewithcylindermesh.h"

#include "openfoam/caseelements/basic/providefields.h"

using namespace insight;

int main(int argc, char*argv[])
{
  return executeTest([=](){

    insight::assertion(argc==2, "expected exactly one command line argument");

    SimpleFoamOpenFOAMCase tc(argv[1]);

    provideFields::Parameters p;
    p.set_createScalarFields
        ( {
            {"T", {0,0,0,1}, 300.0}
          } );
    tc.insert(new provideFields(tc, p));

    tc.runTest();

  });
}
