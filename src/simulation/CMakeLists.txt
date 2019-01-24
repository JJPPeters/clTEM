project(simulation_lib)
cmake_minimum_required( VERSION 3.5 )

cmake_policy(SET CMP0071 NEW)
cmake_policy(SET CMP0074 NEW)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${PROJECT_SOURCE_DIR}/cmake/modules/")

include_directories(clwrapper)
add_subdirectory(clwrapper)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -pthread")

IF(CMAKE_BUILD_TYPE MATCHES DEBUG)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fvar -tracking")
ENDIF(CMAKE_BUILD_TYPE MATCHES DEBUG)

find_package (TIFF REQUIRED)
if(TIFF_FOUND)
    message(STATUS "TIFF found (include: ${TIFF_INCLUDE_DIR})")
endif(TIFF_FOUND)

set(EASYLOGGINGPP_USE_STATIC_LIBS TRUE)
find_package(EASYLOGGINGPP REQUIRED)

if(EASYLOGGINGPP_FOUND)
    message(STATUS "Found EasyLogging++: " ${EASYLOGGINGPP_INCLUDE_DIR})
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DELPP_NO_DEFAULT_LOG_FILE -DELPP_NO_LOG_TO_FILE -DELPP_THREAD_SAFE -DELPP_FORCE_USE_STD_THREAD") #
endif()

set(SIM_HDRS
        simulationmanager.h
        structure/crystalstructure.h
        structure/atom.h
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
        threadpool.cpp threadpool.h simulationjob.cpp simulationjob.h simulationworker.cpp simulationworker.h structure/structureparameters.h kernels.h kernels.cpp clmanager.cpp structure/structureparameters.cpp utilities/fileio.cpp utilities/fileio.h ccdparams.cpp ccdparams.h utilities/enums.h utilities/jsonutils.cpp utilities/jsonutils.h utilities/vectorutils.cpp utilities/vectorutils.h structure/thermalvibrations.cpp structure/thermalvibrations.h utilities/logging.cpp utilities/logging.h)

add_library(simulation STATIC ${SIM_SCRS} ${SIM_HDRS})

target_include_directories (simulation PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} ${EASYLOGGINGPP_INCLUDE_DIR} clwrapper)
target_link_libraries (simulation ${TIFF_LIBRARY} ${EASYLOGGINGPP_LIBRARY} clwrapper)