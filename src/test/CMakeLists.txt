
add_subdirectory(installationTests) # these binaries should go into common bin because they shall be shipped with the install package

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})


if(INSIGHT_BUILD_TESTS)
    add_subdirectory(refdata)
    add_subdirectory(toolkit)
    add_subdirectory(openfoam)
    add_subdirectory(gui)
    add_subdirectory(pdl)
endif()
