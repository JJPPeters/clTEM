project(clwrapper_lib)
cmake_minimum_required( VERSION 3.5 )

# message(STATUS "Project source directory: " ${PROJECT_SOURCE_DIR})
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${PROJECT_SOURCE_DIR}/cmake/modules/")

message(STATUS "${CMAKE_MODULE_PATH}")

find_package(OpenCL REQUIRED)

#set(OpenCL_LIBRARY "D:/Programming/Cpp/AMD_APP_SDK/3.0/lib/x86_64/OpenCL.lib")
#set(OpenCL_INCLUDE_DIRS "D:/Programming/Cpp/AMD_APP_SDK/3.0/include")

if(OpenCL_FOUND)
    message(STATUS "OpenCL include directories: " ${OpenCL_INCLUDE_DIRS})
    message(STATUS "OpenCL library: " ${OpenCL_LIBRARY})
endif()

#set(OpenCL_INCLUDE_DIRS "/opt/AMDAPPSDK-3.0/include")
#set(OpenCL_LIBRARY "/opt/AMDAPPSDK-3.0/lib/x86_64/sdk/libOpenCL.so")

find_package(clFFT REQUIRED)

if(CLFFT_FOUND)
    message(STATUS "clFFT include directories: " ${CLFFT_INCLUDE_DIRS})
    message(STATUS "clFFT library: " ${CLFFT_LIBRARIES})
endif()

find_package(Boost REQUIRED)

if(Boost_FOUND)
    message(STATUS "Boost include directories: " ${Boost_INCLUDE_DIRS})
    message(STATUS "Boost libraries: " ${Boost_LIBRARY_DIRS})
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

add_library(clwrapper STATIC ${SOURCES} ${HEADERS})

target_include_directories (clwrapper PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} ${OpenCL_INCLUDE_DIRS} ${CLFFT_INCLUDE_DIRS} ${Boost_INCLUDE_DIRS})
target_link_libraries (clwrapper ${OpenCL_LIBRARIES} ${CLFFT_LIBRARIES} )