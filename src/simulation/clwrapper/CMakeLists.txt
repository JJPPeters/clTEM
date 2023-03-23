project(clwrapper_lib)
cmake_minimum_required( VERSION 3.5 )

cmake_policy(SET CMP0074 NEW)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${PROJECT_SOURCE_DIR}/cmake/modules/")

set(OpenCL_INCLUDE_DIRS D:/Programming/libraries/OCL_SDK_Light/include)
set(OpenCL_LIBRARY D:/Programming/libraries/OCL_SDK_Light/lib/x86_64/opencl.lib)
set(OpenCL_LIBRARIES D:/Programming/libraries/OCL_SDK_Light/lib/x86_64/opencl.lib)

#find_package(OpenCL REQUIRED)

#if(OpenCL_FOUND)
#    message(STATUS "OpenCL include directories: " ${OpenCL_INCLUDE_DIRS})
#    message(STATUS "OpenCL library: " ${OpenCL_LIBRARY})
#endif()

find_package(clFFT REQUIRED)

if(CLFFT_FOUND)
    message(STATUS "clFFT include directories: " ${CLFFT_INCLUDE_DIRS})
    message(STATUS "clFFT library: " ${CLFFT_LIBRARIES})
endif()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17")

set(HEADERS
        clcontext.h
        cldevice.h
        clerror.h
        clevent.h
        clfourier.h
        clkernel.h
        clmemory.h
        clstatic.h
        clworkgroup.h
        clwrapper.h
        manual.h
        notify.h
        utils.h
        auto.h
        )

set(SOURCES
        cldevice.cpp
        clerror.cpp
        clfourier.cpp
        clkernel.cpp
        clstatic.cpp
        utils.cpp
        )

add_library(clwrapper SHARED ${SOURCES} ${HEADERS})

target_include_directories (clwrapper PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} ${OpenCL_INCLUDE_DIRS} ${CLFFT_INCLUDE_DIRS})
target_link_libraries (clwrapper ${OpenCL_LIBRARIES} ${CLFFT_LIBRARIES} )