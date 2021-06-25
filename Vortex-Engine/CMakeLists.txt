# Vortex-Engine

cmake_minimum_required (VERSION 3.19)
project (Vortex-Engine)

if (CMAKE_BUILD_TYPE MATCHES Debug)
    add_definitions(-DDEBUG)
endif()

add_definitions(-DVT_BUILD_DLL)
add_definitions(-DVT_ENABLE_ASSERTIONS)

set (WIN32_EXECUTABLE TRUE)
set (INCLUDE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/src/Vortex" "${CMAKE_CURRENT_SOURCE_DIR}/vendor")
set (LIBS "opengl32.lib")
file (GLOB_RECURSE SOURCES "src/*/*.cpp")

add_library (${PROJECT_NAME} SHARED ${SOURCES} src/Vortex/Graphics/OpenGL46/GL46Context.cpp src/Vortex/Graphics/OpenGL46/GL46Context.hpp src/Vortex/Core/Assertions.hpp src/Vortex/Graphics/OpenGL46/OpenGL.hpp)

target_include_directories(${PROJECT_NAME} PUBLIC ${INCLUDE_DIRS})
target_link_libraries (${PROJECT_NAME} PUBLIC ${LIBS})

add_custom_command(
        TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different  ${CMAKE_BINARY_DIR}/Vortex-Engine/Vortex-Engine.dll ${CMAKE_BINARY_DIR}/XSandbox/ DEPENDS Vortex-Engine})