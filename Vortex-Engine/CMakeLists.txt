#
# Created by v1tr10l7 on 17.07.2024. Copyright (c) 2024-2024, Szymon Zemke
# <v1tr10l7@proton.me>
#
# SPDX-License-Identifier: GPL-3
#
cmake_minimum_required(VERSION 3.25)
project(Vortex-Engine VERSION 0.1)

cmake_policy(SET CMP0092 NEW)

set(CMAKE_CXX_STANDARD 23)
set(CXX_STANDARD_REQUIRED ON)

option(VORTEX_BUILD_SHARED_LIBS "Build using shared libraries" OFF)
option(VT_BUILD_WAYLAND "Build vortex with wayland support" ON)

find_package(Vulkan REQUIRED)

set(GLFW_BUILD_DOCS
    OFF
    CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS
    OFF
    CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES
    OFF
    CACHE BOOL "" FORCE)
set(SHADERC_SKIP_TESTS
    ON
    CACHE BOOL "" FORCE)
set(SHADERC_SKIP_EXAMPLES
    ON
    CACHE BOOL "" FORCE)
set(SPIRV_SKIP_EXECUTABLES
    ON
    CACHE BOOL "" FORCE)
set(ENABLE_GLSLANG_BINARIES
    OFF
    CACHE BOOL "" FORCE)
set(SPIRV_CROSS_ENABLE_TESTS
    OFF
    CACHE BOOL "" FORCE)
set(SPIRV_CROSS_ENABLE_C_API
    OFF
    CACHE BOOL "" FORCE)
set(SPIRV_CROSS_SKIP_INSTALL
    ON
    CACHE BOOL "" FORCE)
set(SPIRV_CROSS_CLI
    OFF
    CACHE BOOL "" FORCE)
set(SPIRV_CROSS_CLI
    OFF
    CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_TESTS
    OFF
    CACHE BOOL "" FORCE)
set(ASSIMP_WARNINGS_AS_ERRORS
    OFF
    CACHE BOOL "" FORCE)

add_subdirectory(vendor/fmt)
add_subdirectory(vendor/magic_enum)
add_subdirectory(vendor/glm)
add_subdirectory(vendor/glfw)
add_subdirectory(vendor/VulkanMemoryAllocator)
add_subdirectory(vendor/shaderc)
add_subdirectory(vendor/SPIRV-Cross)
add_subdirectory(vendor/assimp)

if(CMAKE_BUILD_TYPE MATCHES Debug)
  add_definitions(-DVT_DEBUG)
elseif(CMAKE_BUILD_TYPE MATCHES RelWithDebInfo)
  add_definitions(-DVT_RELEASE)
elseif(CMAKE_BUILD_TYPE MATCHES Release)
  add_definitions(-DVT_DIST)
endif()

add_definitions(-DVT_ENABLE_ASSERTIONS)
add_definitions(-DGLM_FORCE_RADIANS -DGLM_FORCE_DEPTH_ZERO_TO_ONE)
add_definitions(-DVT_MAX_FRAMES_IN_FLIGHT=2)

set(INCLUDE_DIRS ${INCLUDE_DIRS} "${CMAKE_CURRENT_SOURCE_DIR}/src/Vortex"
                 vendor/imgui vendor/stb/include ${CMAKE_CURRENT_BINARY_DIR})
set(PUBLIC_LIBS
    fmt::fmt-header-only
    magic_enum::magic_enum
    glfw
    glm::glm-header-only
    Vulkan::Vulkan
    Vulkan::Headers
    GPUOpen::VulkanMemoryAllocator
    shaderc
    shaderc_util
    spirv-cross-reflect
    spirv-cross-hlsl
    spirv-cross-glsl
    spirv-cross-cpp
    assimp)

if(UNIX)
  set(PUBLIC_LIBS ${PUBLIC_LIBS} X11 X11-xcb wayland-client xkbcommon)
endif()

set(SOURCES
    src/Vortex/Core/LayerStack.cpp
    src/Vortex/Core/Events/EventSystem.cpp
    src/Vortex/Core/Log/Log.cpp
    src/Vortex/Core/Log/Logger.cpp
    src/Vortex/Engine/Application.cpp
    src/Vortex/Engine/EntryPoint.cpp
    src/Vortex/Renderer/Camera.cpp
    src/Vortex/Renderer/Image.cpp
    src/Vortex/Renderer/Renderer.cpp
    src/Vortex/Renderer/API/IndexBuffer.cpp
    src/Vortex/Renderer/API/GraphicsPipeline.cpp
    src/Vortex/Renderer/API/RendererContext.cpp
    src/Vortex/Renderer/API/Shader.cpp
    src/Vortex/Renderer/API/Texture2D.cpp
    src/Vortex/Renderer/API/UniformBuffer.cpp
    src/Vortex/Renderer/API/VertexBuffer.cpp
    src/Vortex/Renderer/API/Vulkan/VulkanAllocator.cpp
    src/Vortex/Renderer/API/Vulkan/VulkanCommon.cpp
    src/Vortex/Renderer/API/Vulkan/VulkanContext.cpp
    src/Vortex/Renderer/API/Vulkan/VulkanDevice.cpp
    src/Vortex/Renderer/API/Vulkan/VulkanGraphicsPipeline.cpp
    src/Vortex/Renderer/API/Vulkan/VulkanImage.cpp
    src/Vortex/Renderer/API/Vulkan/VulkanImGuiLayer.cpp
    src/Vortex/Renderer/API/Vulkan/VulkanIndexBuffer.cpp
    src/Vortex/Renderer/API/Vulkan/VulkanInstance.cpp
    src/Vortex/Renderer/API/Vulkan/VulkanRenderer.cpp
    src/Vortex/Renderer/API/Vulkan/VulkanShader.cpp
    src/Vortex/Renderer/API/Vulkan/VulkanSurface.cpp
    src/Vortex/Renderer/API/Vulkan/VulkanSwapChain.cpp
    src/Vortex/Renderer/API/Vulkan/VulkanTexture2D.cpp
    src/Vortex/Renderer/API/Vulkan/VulkanUniformBuffer.cpp
    src/Vortex/Renderer/API/Vulkan/VulkanVertexBuffer.cpp
    src/Vortex/Renderer/Window/Monitor.cpp
    src/Vortex/Renderer/Window/Window.cpp
    src/Vortex/Utility/ImageLoader.cpp)

if(UNIX)
  set(SOURCES
      ${SOURCES}
      src/Vortex/Core/Posix/PosixTime.cpp
      src/Vortex/Renderer/Window/Wayland/Wayland.cpp
      src/Vortex/Renderer/Window/Wayland/WaylandWindow.cpp
      src/Vortex/Renderer/Window/Wayland/WaylandMonitor.cpp
      src/Vortex/Renderer/Window/X11/X11Window.cpp
      src/Vortex/Renderer/Window/X11/X11Monitor.cpp)

elseif(WIN32)
  set(SOURCES
      ${SOURCES} src/Vortex/Core/Win32/Win32Time.cpp
      src/Vortex/Renderer/Window/Win32/Win32Window.cpp
      src/Vortex/Renderer/Window/Win32/Win32Monitor.cpp)
endif()

# ImGui
set(SOURCES
    ${SOURCES}
    vendor/imgui/imgui.cpp
    vendor/imgui/imgui_demo.cpp
    vendor/imgui/imgui_draw.cpp
    vendor/imgui/imgui_tables.cpp
    vendor/imgui/imgui_widgets.cpp
    src/Vortex/Renderer/API/Vulkan/imgui_impl_vulkan.cpp)

if(VORTEX_BUILD_SHARED_LIBS)
  add_definitions(-DVT_BUILD_DLL)
  add_library(${PROJECT_NAME} SHARED ${SOURCES})
else()
  add_library(${PROJECT_NAME} STATIC ${SOURCES})
  target_compile_definitions(${PROJECT_NAME} PUBLIC -DVT_STATIC_LIBRARY)
endif()

target_precompile_headers(
  ${PROJECT_NAME} PUBLIC
  "$<$<COMPILE_LANGUAGE:CXX>:${CMAKE_CURRENT_SOURCE_DIR}/src/vtpch.hpp>")

target_compile_options(
  ${PROJECT_NAME}
  PRIVATE $<$<CXX_COMPILER_ID:MSVC>:
          /W4>
          $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall
          -Wextra
          -Wpedantic
          -Werror
          -fvisibility=hidden
          -Wno-nullability-extension
          -Wno-nullability-completeness>)
if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  # Workaround for std::expected not available in clang
  target_compile_options(
    ${PROJECT_NAME}
    PUBLIC $<$<COMPILE_LANGUAGE:CXX>:-stdlib=libstdc++ -D__cpp_concepts=202002
           -Wno-builtin-macro-redefined -Wno-unused-command-line-argument>)
endif()

if(VT_BUILD_WAYLAND AND UNIX)
  macro(generate_wayland_protocol protocol_file)
    set(protocol_path
        "${CMAKE_CURRENT_SOURCE_DIR}/vendor/wayland-protocols/${protocol_file}")

    string(REGEX REPLACE "\\.xml$" "-client-protocol.h" header_file
                         ${protocol_file})
    string(REGEX REPLACE "\\.xml$" "-client-protocol-code.c" code_file
                         ${protocol_file})
    set(header_file wayland-${header_file})
    set(code_file wayland-${code_file})

    add_custom_command(
      OUTPUT ${header_file}
      COMMAND "${WAYLAND_SCANNER_EXECUTABLE}" -c client-header
              "${protocol_path}" ${header_file}
      DEPENDS "${protocol_path}"
      VERBATIM)

    add_custom_command(
      OUTPUT ${code_file}
      COMMAND "${WAYLAND_SCANNER_EXECUTABLE}" private-code "${protocol_path}"
              ${code_file}
      DEPENDS "${protocol_path}"
      VERBATIM)

    target_sources(${PROJECT_NAME} PRIVATE ${header_file} ${code_file})
  endmacro()

  generate_wayland_protocol("wayland.xml")
  generate_wayland_protocol("xdg-shell.xml")
  generate_wayland_protocol("alpha-modifier-v1.xml")
  generate_wayland_protocol("wlr-gamma-control-unstable-v1.xml")
  generate_wayland_protocol("xdg-toplevel-icon-v1.xml")
endif()

target_include_directories(${PROJECT_NAME} PRIVATE ${INCLUDE_DIRS})
target_include_directories(${PROJECT_NAME} PUBLIC vendor/stb/include)
target_include_directories(${PROJECT_NAME}
                           PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src vendor/imgui)
target_link_libraries(${PROJECT_NAME} PUBLIC ${PUBLIC_LIBS})
target_compile_features(${PROJECT_NAME} INTERFACE cxx_std_23)

if(WIN32)
  target_link_libraries(${PROJECT_NAME} PUBLIC Shcore)
endif()
