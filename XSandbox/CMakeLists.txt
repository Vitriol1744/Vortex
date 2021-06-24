# XSandbox

cmake_minimum_required (VERSION 3.19)
project (XSandbox)

file (GLOB_RECURSE SOURCES "src/*.cpp")

if (CMAKE_BUILD_TYPE MATCHES Debug)
    add_definitions(-DDEBUG)
endif()

add_definitions(-DVT_ENABLE_ASSERTIONS)

add_executable (XSandbox ${SOURCES})

target_link_libraries (XSandbox PUBLIC Vortex-Engine)
include_directories ("${CMAKE_CURRENT_SOURCE_DIR}/../Vortex-Engine/src")

#file(COPY ${CMAKE_BINARY_DIR}/Vortex-Engine/Vortex-Engine.dll DESTINATION "${CMAKE_BINARY_DIR}/XSandbox")