# Vortex

cmake_minimum_required (VERSION 3.10)
project (Vortex)

add_link_options(-fPIC)
add_compile_options(-fPIC)

set (CMAKE_CXX_STANDARD 23)
set (CXX_STANDARD_REQUIRED ON)

if (CMAKE_BUILD_TYPE MATCHES Debug)
    add_definitions (-DDEBUG)
endif()

add_definitions (-DUSE_VULKAN)

add_subdirectory (Vortex-Engine)
add_subdirectory (XSandbox)