cmake_minimum_required (VERSION 3.16)

project (glad LANGUAGES C)

set ( CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -Wall --std=c++20 -O3 -fPIC")
set ( CMAKE_C_FLAGS  "${CMAKE_C_FLAGS} -Wall -O3 -fPIC")

add_library(${PROJECT_NAME} STATIC src/glad.c)

target_include_directories (${PROJECT_NAME} PUBLIC include/)