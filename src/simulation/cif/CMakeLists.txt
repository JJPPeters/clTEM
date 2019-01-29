project(cif_lib)
cmake_minimum_required( VERSION 3.5 )

cmake_policy(SET CMP0074 NEW)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${PROJECT_SOURCE_DIR}/cmake/modules/")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")

IF(CMAKE_BUILD_TYPE MATCHES DEBUG)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fvar -tracking")
ENDIF(CMAKE_BUILD_TYPE MATCHES DEBUG)

add_definitions ( -Wall )

find_package (Eigen3 REQUIRED)
if(EIGEN3_FOUND)
    message(STATUS "Found EIGEN3: " ${EIGEN3_INCLUDE_DIR})
endif(EIGEN3_FOUND)

set(HEADERS
        atomsite.h
        cellgeometry.h
        cifreader.h
        cifutilities.h
        symmetryoperation.h
        unitcell.h
        supercell.h
        )

set(SOURCES
        atomsite.cpp
        cellgeometry.cpp
        cifreader.cpp
        symmetryoperation.cpp
        supercell.cpp
        )

add_library(cif SHARED ${SOURCES} ${HEADERS})
target_include_directories (cif PUBLIC ${EIGEN3_INCLUDE_DIR})