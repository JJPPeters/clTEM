project( clTEM_cmd )
cmake_minimum_required( VERSION 3.5 )

cmake_policy(SET CMP0071 NEW)
cmake_policy(SET CMP0074 NEW)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")

IF(CMAKE_BUILD_TYPE MATCHES DEBUG)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fvar -tracking")
ENDIF(CMAKE_BUILD_TYPE MATCHES DEBUG)

add_definitions ( -Wall )

include_directories(simulation)

set ( clTEM_HDRS
        )

set ( clTEM_SRCS
        main.cpp
        parseopencl.h)

find_package(Boost COMPONENTS filesystem system)

if(Boost_FOUND)
    message(STATUS "Boost include directories: " ${Boost_INCLUDE_DIRS})
    message(STATUS "Boost libraries: " ${Boost_LIBRARY_DIRS})
endif()

add_executable ( clTEM_cmd ${clTEM_SRCS})
target_include_directories(clTEM_cmd PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} simulation)
target_link_libraries ( clTEM_cmd ${Boost_FILESYSTEM_LIBRARY} ${Boost_SYSTEM_LIBRARY} simulation )
