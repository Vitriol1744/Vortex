# Vortex

cmake_minimum_required (VERSION 3.16)
project (Vortex)

set (CMAKE_CXX_STANDARD 20)

set (CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} -fPIC)

add_subdirectory (Vortex-Engine)
add_subdirectory (XSandbox)