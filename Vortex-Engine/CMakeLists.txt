# 
#  Created by v1tr10l7 on 17.07.2024.
#  Copyright (c) 2024-2024, Szymon Zemke <v1tr10l7@proton.me>
# 
#  SPDX-License-Identifier: GPL-3
#
cmake_minimum_required (VERSION 3.15)
project (Vortex-Engine)

cmake_policy(SET CMP0092 NEW)

set (CMAKE_CXX_STANDARD 23)
set (CXX_STANDARD_REQUIRED ON)

option(VORTEX_BUILD_SHARED_LIBS "Build using shared libraries" OFF)

find_package (Vulkan REQUIRED)
find_package (VulkanHeaders CONFIG)

add_subdirectory (vendor/fmt)
add_subdirectory (vendor/magic_enum)
add_subdirectory (vendor/glm)
add_subdirectory (vendor/glfw)

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

if (CMAKE_BUILD_TYPE MATCHES Debug)
    add_definitions (-DVT_DEBUG)
elseif (CMAKE_BUILD_TYPE MATCHES RelWithDebInfo)
    add_definitions (-DVT_RELEASE)
elseif (CMAKE_BUILD_TYPE MATCHES Release)
    add_definitions (-DVT_DIST)
endif ()

add_definitions (-DVT_ENABLE_ASSERTIONS)

set (INCLUDE_DIRS ${INCLUDE_DIRS} "${CMAKE_CURRENT_SOURCE_DIR}/src/Vortex" vendor/imgui)
set (LIBS ${Vulkan_LIBRARIES} fmt::fmt-header-only magic_enum::magic_enum glfw glm::glm-header-only Vulkan::Headers)

set (SOURCES
    src/Vortex/Core/LayerStack.cpp
    src/Vortex/Core/Events/EventSystem.cpp
    src/Vortex/Core/Log/Log.cpp 
    src/Vortex/Core/Log/Logger.cpp
    src/Vortex/Engine/Application.cpp
    src/Vortex/Engine/EntryPoint.cpp
    src/Vortex/Renderer/API/GraphicsPipeline.cpp
    src/Vortex/Renderer/API/RendererContext.cpp
    src/Vortex/Renderer/API/Shader.cpp
    src/Vortex/Renderer/API/Vulkan/VulkanCommon.cpp
    src/Vortex/Renderer/API/Vulkan/VulkanContext.cpp
    src/Vortex/Renderer/API/Vulkan/VulkanDevice.cpp
    src/Vortex/Renderer/API/Vulkan/VulkanGraphicsPipeline.cpp
    src/Vortex/Renderer/API/Vulkan/VulkanImGuiLayer.cpp
    src/Vortex/Renderer/API/Vulkan/VulkanInstance.cpp
    src/Vortex/Renderer/API/Vulkan/VulkanShader.cpp
    src/Vortex/Renderer/API/Vulkan/VulkanSurface.cpp
    src/Vortex/Renderer/API/Vulkan/VulkanSwapChain.cpp
    src/Vortex/Renderer/Window/Monitor.cpp
    src/Vortex/Renderer/Window/Window.cpp
)

if (UNIX)
    set (SOURCES ${SOURCES}
        src/Vortex/Renderer/Window/X11/X11Window.cpp
        src/Vortex/Renderer/Window/X11/X11Monitor.cpp)
elseif (WIN32)
    set (SOURCES ${SOURCES}
        src/Vortex/Renderer/Window/Win32/Win32Window.cpp
        src/Vortex/Renderer/Window/Win32/Win32Monitor.cpp)
endif ()

# ImGui
set (SOURCES ${SOURCES} 
    vendor/imgui/imgui.cpp 
    vendor/imgui/imgui_demo.cpp 
    vendor/imgui/imgui_draw.cpp
    vendor/imgui/imgui_tables.cpp
    vendor/imgui/imgui_widgets.cpp
    
    src/Vortex/Renderer/API/Vulkan/imgui_impl_vulkan.cpp)
    
if (VORTEX_BUILD_SHARED_LIBS)
    add_definitions (-DVT_BUILD_DLL)
    add_library (${PROJECT_NAME} SHARED ${SOURCES})
else ()
    add_library (${PROJECT_NAME} STATIC ${SOURCES})
    target_compile_definitions (${PROJECT_NAME} PUBLIC -DVT_STATIC_LIBRARY)
endif ()

target_precompile_headers(${PROJECT_NAME} PUBLIC src/vtpch.hpp)

target_compile_options(${PROJECT_NAME} PRIVATE
  $<$<CXX_COMPILER_ID:MSVC>: /W4>
  $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra -Wpedantic -Werror -fvisibility=hidden>
)

target_include_directories(${PROJECT_NAME} PRIVATE ${INCLUDE_DIRS})
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src vendor/imgui)
target_link_libraries (${PROJECT_NAME} PUBLIC ${LIBS})
