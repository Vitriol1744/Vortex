# Vortex-Engine

cmake_minimum_required (VERSION 3.19)
project (Vortex-Engine)

if (CMAKE_BUILD_TYPE MATCHES Debug)
    add_definitions(-DDEBUG)
endif()

add_definitions(-DVT_BUILD_DLL)
add_definitions(-DVT_ENABLE_ASSERTIONS)

set (WIN32_EXECUTABLE TRUE)

file (GLOB_RECURSE SOURCES "src/*/*.cpp")

add_library (Vortex-Engine SHARED ${SOURCES})

target_include_directories(${PROJECT_NAME} PUBLIC "K:/Dev/C++/Vortex/Vortex-Engine/src/Vortex")

add_custom_command(
        TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different  ${CMAKE_BINARY_DIR}/Vortex-Engine/Vortex-Engine.dll ${CMAKE_BINARY_DIR}/XSandbox/ DEPENDS Vortex-Engine})