project(simulation_lib)
cmake_minimum_required( VERSION 3.5 )

cmake_policy(SET CMP0071 NEW)
cmake_policy(SET CMP0074 NEW)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${PROJECT_SOURCE_DIR}/cmake/modules/")

# Add a custom command that produces version.cpp, plus
# a dummy output that's not actually produced, in order
# to force version.cmake to always be re-run before the build
ADD_CUSTOM_COMMAND(
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/version.cpp
        ${CMAKE_CURRENT_BINARY_DIR}/_version.cpp
        COMMAND ${CMAKE_COMMAND} -P
        ${CMAKE_CURRENT_SOURCE_DIR}/cmake/version.cmake)

# this is important as it will be used by the installer scripts
configure_file(version.cpp version.cpp COPYONLY)
#

include_directories(clwrapper)
add_subdirectory(clwrapper)

include_directories(cif)
add_subdirectory(cif)

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

find_package (ModernJson REQUIRED)
if(JSON_FOUND)
    message(STATUS "Found Modern JSON for c++: " ${JSON_INCLUDE_DIRS})
endif(JSON_FOUND)

set(SIM_HDRS
        simulationmanager.h
        kernels.h
        ccdparams.h
        #
        structure/crystalstructure.h
        structure/atom.h
        structure/structureparameters.h
        #
        incoherence/incoherenteffects.h
        incoherence/inelastic/plasmon.h
        incoherence/inelastic/phonon.h
        #
        utilities/commonstructs.h
        utilities/stringutils.h
        utilities/structureutils.h
        utilities/fileio.h
        utilities/enums.h
        utilities/jsonutils.h
        utilities/vectorutils.h
        utilities/logging.h
        utilities/simutils.h
        #
        threading/simulationrunner.h
        threading/threadpool.h
        threading/threadworker.h
        threading/simulationjob.h
        #
        microscope/simulationgeneral.h
        microscope/simulationworker.h
        microscope/simulationctem.h
        microscope/simulationcbed.h
        microscope/simulationstem.h
        structure/simulationcell.h
        incoherence/probesourcesize.h
        incoherence/chromaticaberration.h
        utilities/getversion.h
        )

set(SIM_SCRS
        simulationmanager.cpp
        kernels.cpp
        ccdparams.cpp
        #
        structure/crystalstructure.cpp
        structure/structureparameters.cpp
        #
        incoherence/incoherenteffects.cpp
        incoherence/inelastic/plasmon.cpp
        incoherence/inelastic/phonon.cpp
        #
        utilities/commonstructs.cpp
        utilities/stringutils.cpp
        utilities/structureutils.cpp
        utilities/fileio.cpp
        utilities/jsonutils.cpp
        utilities/vectorutils.cpp
        utilities/logging.cpp
        utilities/simutils.cpp
        #
        threading/simulationrunner.cpp
        threading/threadpool.cpp
        threading/threadworker.cpp
        threading/simulationjob.cpp
        #
        microscope/simulationgeneral.cpp
        microscope/simulationworker.cpp
        microscope/simulationctem.cpp
        microscope/simulationcbed.cpp
        microscope/simulationstem.cpp
        structure/simulationcell.cpp
        incoherence/probesourcesize.cpp
        incoherence/chromaticaberration.cpp
        utilities/getversion.cpp
        ${CMAKE_CURRENT_BINARY_DIR}/version.cpp)

add_library(simulation STATIC ${SIM_SCRS} ${SIM_HDRS})

target_include_directories (simulation SYSTEM PUBLIC ${EASYLOGGINGPP_INCLUDE_DIR} ${JSON_INCLUDE_DIRS})
target_include_directories (simulation PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} clwrapper cif)
target_link_libraries (simulation ${TIFF_LIBRARY} ${EASYLOGGINGPP_LIBRARY} clwrapper cif)