project(simulation_lib)
cmake_minimum_required( VERSION 3.5 )

include_directories(clwrapper)
add_subdirectory(clwrapper)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -pthread")

IF(CMAKE_BUILD_TYPE MATCHES DEBUG)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fvar -tracking")
ENDIF(CMAKE_BUILD_TYPE MATCHES DEBUG)

find_package (TIFF REQUIRED)
if(TIFF_FOUND)
    message(STATUS "TIFF found (include: ${TIFF_INCLUDE_DIR})")
endif(TIFF_FOUND)

set(SIM_HDRS
        simulationmanager.h
        structure/crystalstructure.h
        utilities/commonstructs.h
        utilities/stringutils.h
        utilities/structureutils.h
        clmanager.h
        threadworker.h
        )

set(SIM_SCRS
        simulationmanager.cpp
        structure/crystalstructure.cpp
        utilities/commonstructs.cpp
        utilities/stringutils.cpp
        utilities/structureutils.cpp
        simulationrunner.cpp
#        simulations/ctemsimulation.cpp
#        simulations/stemsimulation.cpp
#        simulations/microscopesimulation.cpp
        threadworker.cpp
        threadpool.cpp threadpool.h simulationjob.cpp simulationjob.h simulationworker.cpp simulationworker.h structure/structureparameters.h kernels.h kernels.cpp clmanager.cpp structure/structureparameters.cpp utilities/fileio.cpp utilities/fileio.h)

add_library(simulation STATIC ${SIM_SCRS} ${SIM_HDRS} )

target_include_directories (simulation PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} clwrapper)
target_link_libraries (simulation ${TIFF_LIBRARY} clwrapper)