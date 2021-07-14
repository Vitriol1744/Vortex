# Vortex-Engine

cmake_minimum_required (VERSION 3.16)
project (Vortex-Engine)

find_package(OpenGL REQUIRED COMPONENTS OpenGL)

#TODO: Download FMT

add_subdirectory (vendor/fmt)

if (CMAKE_BUILD_TYPE MATCHES Debug)
    add_definitions(-DDEBUG)
endif()

add_definitions(-DVT_BUILD_DLL)
add_definitions(-DVT_ENABLE_ASSERTIONS)

set (WIN32_EXECUTABLE TRUE)
set (INCLUDE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/src/Vortex" "${CMAKE_CURRENT_SOURCE_DIR}/vendor" "fmt")
set (LIBS "OpenGL::GL" "fmt")
file (GLOB_RECURSE SOURCES "src/*/*.cpp")

if (UNIX)
set (LIBS ${LIBS} X11)
endif()

add_library (${PROJECT_NAME} STATIC ${SOURCES} src/Vortex/Core/Math/Matrix.cpp src/Vortex/Core/Math/MathCommon.hpp src/Vortex/Core/Math/MathCommon.cpp src/Vortex/Core/IEvent.hpp src/Vortex/Core/EventSystem.cpp src/Vortex/Core/EventSystem.hpp src/Vortex/Events/Events.hpp src/Vortex/Input/KeyCode.hpp src/Vortex/Input/MouseCode.hpp src/Vortex/Graphics/VertexBufferLayout.hpp src/Vortex/Graphics/IMesh.cpp src/Vortex/Graphics/IMesh.hpp src/Vortex/Graphics/OpenGL46/GL46Mesh.cpp src/Vortex/Graphics/OpenGL46/GL46Mesh.hpp src/Vortex/Graphics/OpenGL46/OpenGL.cpp src/Vortex/Graphics/Graphics.hpp src/Vortex/Graphics/IShader.cpp src/Vortex/Graphics/IShader.hpp src/Vortex/Graphics/OpenGL46/GL46Shader.cpp src/Vortex/Graphics/OpenGL46/GL46Shader.hpp)

target_include_directories(${PROJECT_NAME} PUBLIC ${INCLUDE_DIRS})
target_link_libraries (${PROJECT_NAME} PUBLIC ${LIBS})
#target_precompile_headers (${PROJECT_NAME} PUBLIC "src/Vortex/vtpch.hpp")

#if (_WIN32)
#add_custom_command(
#TARGET ${PROJECT_NAME} POST_BUILD
#COMMAND ${CMAKE_COMMAND} -E copy_if_different  ${CMAKE_BINARY_DIR}/Vortex-Engine/Vortex-Engine.dll ${CMAKE_BINARY_DIR}/XSandbox/ DEPENDS Vortex-Engine})
#elseif (UNIX)
#add_custom_command(
#TARGET ${PROJECT_NAME} POST_BUILD
#COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_BINARY_DIR}/Vortex-Engine/libVortex-Engine.so ${CMAKE_BINARY_DIR}/XSandbox/ DEPENDS Vortex-Engine})
#endif ()

add_custom_command(
TARGET ${PROJECT_NAME} POST_BUILD
COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/assets ${CMAKE_BINARY_DIR}/XSandbox/assets)
