# Vortex-Engine

cmake_minimum_required (VERSION 3.16)
project (Vortex-Engine)

find_package(OpenGL REQUIRED COMPONENTS OpenGL)

add_subdirectory (vendor/glad)
add_subdirectory (vendor/fmt)

if (CMAKE_BUILD_TYPE MATCHES Debug)
    add_definitions(-DDEBUG)
endif()

add_definitions(-DVT_BUILD_DLL)
add_definitions(-DVT_ENABLE_ASSERTIONS)

set (WIN32_EXECUTABLE TRUE)
set (INCLUDE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/src/Vortex" "${CMAKE_CURRENT_SOURCE_DIR}/vendor" "fmt")
set (LIBS "OpenGL::GL" "glad" "fmt")
file (GLOB_RECURSE SOURCES "src/*/*.cpp")

add_library (${PROJECT_NAME} SHARED ${SOURCES})

target_include_directories(${PROJECT_NAME} PUBLIC ${INCLUDE_DIRS})
target_link_libraries (${PROJECT_NAME} PUBLIC ${LIBS})
target_precompile_headers (${PROJECT_NAME} PUBLIC "src/Vortex/vtpch.hpp")

if (_WIN32)
add_custom_command(
        TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different  ${CMAKE_BINARY_DIR}/Vortex-Engine/Vortex-Engine.dll ${CMAKE_BINARY_DIR}/XSandbox/ DEPENDS Vortex-Engine})
endif ()