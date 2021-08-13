# Vortex

cmake_minimum_required (VERSION 3.10)
project (Vortex)

add_link_options(-fPIC)
add_compile_options(-fPIC)

set (CMAKE_CXX_STANDARD 20)
set (CXX_STANDARD_REQUIRED ON)

add_subdirectory (Vortex-Engine)
add_subdirectory (XSandbox)