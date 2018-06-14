project( clTEM_gui )
cmake_minimum_required( VERSION 3.5 )

# Note this is needed else the cl_float2 does not have the .x or .y members
set(CMAKE_CXX_STANDARD 17)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

add_subdirectory(gui)
add_subdirectory(console)
add_subdirectory(simulation)
