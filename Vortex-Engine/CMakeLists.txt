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

include(cmake/Dependencies.cmake)

option(VORTEX_BUILD_SHARED_LIBS "Build using shared libraries" OFF)
option(VT_BUILD_WAYLAND "Build vortex with wayland support" ON)

set (PUBLIC_DEFINES 
    -DVT_ENABLE_ASSERTIONS
    -DPRISM_USE_NAMESPACE=1
    -DFMT_HEADER_ONLY=1
    -DGLM_FORCE_RADIANS 
    -DGLM_FORCE_DEPTH_ZERO_TO_ONE
    -DGLM_ENABLE_EXPERIMENTAL)

set(INCLUDE_DIRS ${INCLUDE_DIRS} "${CMAKE_CURRENT_SOURCE_DIR}/Source/Vortex"
    vendor/imgui vendor/stb/include ${CMAKE_CURRENT_BINARY_DIR} vendor/Prism/Source)


find_package(PkgConfig REQUIRED)
pkg_check_modules(PRISM REQUIRED IMPORTED_TARGET prism)
set(PUBLIC_LIBS
    # fmt::fmt-header-only
    Coral.Native
    magic_enum::magic_enum
    prism
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
    assimp
    nlohmann_json::nlohmann_json
    nfd)

if(NOT CMAKE_BUILD_TYPE MATCHES Release)
  set(PUBLIC_LIBS ${PUBLIC_LIBS} Tracy::TracyClient)
endif()
if(UNIX)
  set(PUBLIC_LIBS ${PUBLIC_LIBS} xcb X11 X11-xcb xkbcommon wayland-client)
endif()

set(SOURCES
    Source/Vortex/Asset/AssetRegistry.cpp
    Source/Vortex/Asset/AssetManager.cpp
    Source/Vortex/Asset/ShaderLibrary.cpp
    Source/Vortex/Core/LayerStack.cpp
    Source/Vortex/Core/Events/EventSystem.cpp
    Source/Vortex/Core/Log/Log.cpp
    Source/Vortex/Core/Log/Logger.cpp
    Source/Vortex/Engine/Application.cpp
    Source/Vortex/Engine/EntryPoint.cpp
    Source/Vortex/Filesystem/Filesystem.cpp
    Source/Vortex/Renderer/Camera.cpp
    Source/Vortex/Renderer/Image.cpp
    Source/Vortex/Renderer/Mesh.cpp
    Source/Vortex/Renderer/Renderer.cpp
    Source/Vortex/Renderer/Renderer2D.cpp
    Source/Vortex/Renderer/SceneRenderer.cpp
    Source/Vortex/Renderer/API/IndexBuffer.cpp
    Source/Vortex/Renderer/API/Framebuffer.cpp
    Source/Vortex/Renderer/API/GraphicsPipeline.cpp
    Source/Vortex/Renderer/API/Material.cpp
    Source/Vortex/Renderer/API/Shader.cpp
    Source/Vortex/Renderer/API/SwapChain.cpp
    Source/Vortex/Renderer/API/Texture2D.cpp
    Source/Vortex/Renderer/API/UniformBuffer.cpp
    Source/Vortex/Renderer/API/VertexBuffer.cpp
    Source/Vortex/Renderer/API/Vulkan/VulkanAllocator.cpp
    Source/Vortex/Renderer/API/Vulkan/VulkanCommon.cpp
    Source/Vortex/Renderer/API/Vulkan/VulkanDevice.cpp
    Source/Vortex/Renderer/API/Vulkan/VulkanFramebuffer.cpp
    Source/Vortex/Renderer/API/Vulkan/VulkanGraphicsPipeline.cpp
    Source/Vortex/Renderer/API/Vulkan/VulkanImage.cpp
    Source/Vortex/Renderer/API/Vulkan/VulkanImGuiLayer.cpp
    Source/Vortex/Renderer/API/Vulkan/VulkanIndexBuffer.cpp
    Source/Vortex/Renderer/API/Vulkan/VulkanInstance.cpp
    Source/Vortex/Renderer/API/Vulkan/VulkanMaterial.cpp
    Source/Vortex/Renderer/API/Vulkan/VulkanRenderer.cpp
    Source/Vortex/Renderer/API/Vulkan/VulkanShader.cpp
    Source/Vortex/Renderer/API/Vulkan/VulkanSurface.cpp
    Source/Vortex/Renderer/API/Vulkan/VulkanSwapChain.cpp
    Source/Vortex/Renderer/API/Vulkan/VulkanTexture2D.cpp
    Source/Vortex/Renderer/API/Vulkan/VulkanUniformBuffer.cpp
    Source/Vortex/Renderer/API/Vulkan/VulkanVertexBuffer.cpp
    Source/Vortex/Renderer/ImGui/ImGui.cpp
    Source/Vortex/Scene/Scene.cpp
    Source/Vortex/Script/ScriptEngine.cpp
    Source/Vortex/Utility/ImageLoader.cpp
    Source/Vortex/Utility/Image.cpp
    Source/Vortex/Utility/ZLib.cpp
    Source/Vortex/Window/Monitor.cpp
    Source/Vortex/Window/Window.cpp
    Source/Vortex/Window/Input/Keyboard.cpp
    Source/Vortex/Window/Input/Mouse.cpp)

if(UNIX)
  set(SOURCES
      ${SOURCES}
      Source/Vortex/Core/Memory/Posix/PosixMemory.cpp
      Source/Vortex/Core/Posix/PosixTime.cpp
      Source/Vortex/Window/Wayland/Wayland.cpp
      Source/Vortex/Window/Wayland/WaylandWindow.cpp
      Source/Vortex/Window/Wayland/WaylandMonitor.cpp
      Source/Vortex/Window/X11/X11.cpp
      Source/Vortex/Window/X11/X11Monitor.cpp
      Source/Vortex/Window/X11/X11Window.cpp
      Source/Vortex/Window/X11/XkbUnicode.cpp)

elseif(WIN32)
  set(SOURCES
      ${SOURCES} 
      Source/Vortex/Core/Memory/Win32/Win32Memory.cpp
      Source/Vortex/Core/Win32/Win32Time.cpp
      Source/Vortex/Window/Win32/Win32Window.cpp
      Source/Vortex/Window/Win32/Win32Monitor.cpp)
endif()

# ImGui
set(SOURCES
    ${SOURCES}
    vendor/imgui/imgui.cpp
    vendor/imgui/imgui_demo.cpp
    vendor/imgui/imgui_draw.cpp
    vendor/imgui/imgui_tables.cpp
    vendor/imgui/imgui_widgets.cpp
    Source/Vortex/Renderer/API/Vulkan/imgui_impl_vulkan.cpp
    vendor/ImGuizmo/ImGuizmo.cpp)

# Prism
set (SOURCES
    vendor/Prism/Source/Prism/Algorithm/Hash.cpp

    vendor/Prism/Source/Prism/Containers/Bitmap.cpp
    vendor/Prism/Source/Prism/Containers/RingBuffer.cpp

    vendor/Prism/Source/Prism/Debug/Assertions.cpp
    vendor/Prism/Source/Prism/Debug/Log.cpp
    vendor/Prism/Source/Prism/Debug/Logger.cpp
    vendor/Prism/Source/Prism/Debug/LogSink.cpp
    vendor/Prism/Source/Prism/Debug/Ubsan.cpp
    vendor/Prism/Source/Prism/Debug/Stacktrace.cpp

    vendor/Prism/Source/Prism/Memory/Memory.cpp
    vendor/Prism/Source/Prism/String/Printf.cpp

    vendor/Prism/Source/Prism/Utility/Path.cpp
    vendor/Prism/Source/Prism/Utility/PathView.cpp
    vendor/Prism/Source/Prism/Utility/Path.cpp
)

if(VORTEX_BUILD_SHARED_LIBS)
  set(PUBLIC_DEFINES ${PUBLIC_DEFINES} -DVT_BUILD_DLL)
  add_library(${PROJECT_NAME} SHARED ${SOURCES})
else()
  add_library(${PROJECT_NAME} STATIC ${SOURCES})
  set(PUBLIC_DEFINES ${PUBLIC_DEFINES} -DVT_STATIC_LIBRARY)
endif()

target_precompile_headers(
  ${PROJECT_NAME} PUBLIC
  "$<$<COMPILE_LANGUAGE:CXX>:${CMAKE_CURRENT_SOURCE_DIR}/Source/vtpch.hpp>")

target_compile_options(
  ${PROJECT_NAME}
  PRIVATE $<$<CXX_COMPILER_ID:MSVC>:
          /W4>
          $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:
          -Wall
          -Wextra
          -Werror
          -fvisibility=hidden
          -Wno-nullability-extension
          -Wno-nontrivial-memcall
          -Wno-uninitialized-const-pointer
          -Wno-nullability-completeness
          -Wno-nested-anon-types>)
      if(CMAKE_CXX_COMPILER_ID MATCHES "Clang" OR CMAKE_CXX_COMPILER_ID MATCHES "GCC")
  # Workaround for std::expected not available in clang
  target_compile_options(
    ${PROJECT_NAME}
    PUBLIC $<$<COMPILE_LANGUAGE:CXX>:-stdlib=libstdc++ -D__cpp_concepts=202002
    -Wno-builtin-macro-redefined -Wno-unused-command-line-argument -Wno-deprecated-literal-operator -Wno-variadic-macros>)
endif()
target_compile_options(
  ${PROJECT_NAME}
  PUBLIC -Wno-variadic-macros)

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
target_include_directories(
  ${PROJECT_NAME} PUBLIC vendor/stb/include vendor/entt/single_include)
target_include_directories(
  ${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/Source vendor/imgui
                         vendor/ImGuizmo)
target_link_libraries(${PROJECT_NAME} PUBLIC ${PUBLIC_LIBS})
if(CMAKE_BUILD_TYPE MATCHES Debug OR CMAKE_BUILD_TYPE MATCHES RelWithDebInfo)
  target_compile_options(${PROJECT_NAME} PUBLIC -fsanitize=address)
  target_link_options(${PROJECT_NAME} PUBLIC -fsanitize=address)

  if(CMAKE_BUILD_TYPE MATCHES Debug)
    set(PUBLIC_DEFINES ${PUBLIC_DEFINES} -DVT_DEBUG)
  elseif(CMAKE_BUILD_TYPE MATCHES RelWithDebInfo)
    set(PUBLIC_DEFINES ${PUBLIC_DEFINES} -DVT_RELEASE)
  endif()
elseif(CMAKE_BUILD_TYPE MATCHES Release)
    set(PUBLIC_DEFINES ${PUBLIC_DEFINES} -DVT_DIST)
endif()
target_compile_features(${PROJECT_NAME} INTERFACE cxx_std_23)
target_compile_definitions(
  ${PROJECT_NAME} PUBLIC ${PUBLIC_DEFINES})

if(WIN32)
  target_link_libraries(${PROJECT_NAME} PUBLIC Shcore)
endif()
