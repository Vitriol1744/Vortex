# XSandbox

cmake_minimum_required (VERSION 3.16)
project (XSandbox)

file (GLOB_RECURSE SOURCES "src/*.cpp")

if (CMAKE_BUILD_TYPE MATCHES Debug)
    add_definitions(-DDEBUG)
endif()

set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
add_definitions(-DVT_ENABLE_ASSERTIONS)

add_executable (XSandbox ${SOURCES})

target_link_libraries (XSandbox PUBLIC Vortex-Engine)
include_directories ("${CMAKE_CURRENT_SOURCE_DIR}/../Vortex-Engine/src" Vortex-Engine)