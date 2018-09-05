project( glViewWidget_lib )
cmake_minimum_required( VERSION 3.5 )

cmake_policy(SET CMP0071 NEW)
cmake_policy(SET CMP0072 NEW)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${PROJECT_SOURCE_DIR}/cmake/modules/")

set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")

IF(CMAKE_BUILD_TYPE MATCHES DEBUG)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fvar -tracking")
ENDIF(CMAKE_BUILD_TYPE MATCHES DEBUG)

add_definitions ( -Wall )

find_package(Qt5Widgets REQUIRED)
find_package(Qt5PrintSupport REQUIRED)
find_package(Qt5OpenGL REQUIRED)

if(Qt5Widgets_FOUND)
    message(STATUS "Found Qt version: " ${Qt5Widgets_VERSION})
endif()

include_directories(${Qt5Widgets_INCLUDE_DIRS})
add_definitions(${Qt5Widgets_DEFINITIONS})

include_directories(${Qt5PrintSupport_INCLUDE_DIRS})
add_definitions(${Qt5PrintSupport_DEFINITIONS})

include_directories(${Qt5POpenGL_INCLUDE_DIRS})
add_definitions(${Qt5OpenGL_DEFINITIONS})

set(oglViewWidgets_RSCS shaders/shaders.qrc)
QT5_ADD_RESOURCES(RSCS ${oglViewWidgets_RSCS})

find_package(OpenGL REQUIRED)

if(OpenGL_FOUND)
    message(STATUS "Found OpenGL: " ${OPENGL_LIBRARIES})
endif()

find_package (Eigen3 REQUIRED)
if(EIGEN3_FOUND)
    message(STATUS "Found EIGEN3: " ${EIGEN3_INCLUDE_DIR})
endif(EIGEN3_FOUND)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${Qt5Widgets_EXECUTABLE_COMPILE_FLAGS} ${Qt5PrintSupport_EXECUTABLE_COMPILE_FLAGS}")

set ( glViewWidget_HDRS
        oglbillboardtechnique.h
        oglcamera.h
        oglcamerapipeline.h
        ogldefines.h
        oglmaths.h
        oglrectangletechnique.h
        ogltechnique.h
        ogltexture.h
        oglvertexbuffer.h
        oglviewwidget.h
        )

set ( glViewWidget_SRCS
        oglbillboardtechnique.cpp
        oglcamera.cpp
        oglcamerapipeline.cpp
        ogldefines.cpp
        oglmaths.cpp
        oglrectangletechnique.cpp
        ogltechnique.cpp
        ogltexture.cpp
        oglvertexbuffer.cpp
        oglviewwidget.cpp
        )

add_library ( glViewWidget STATIC ${glViewWidget_HDRS} ${glViewWidget_SRCS} ${RSCS})
target_include_directories(glViewWidget PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} ${EIGEN3_INCLUDE_DIR})
target_link_libraries ( glViewWidget  Qt5::Widgets Qt5::PrintSupport Qt5::OpenGL ${OPENGL_LIBRARIES})