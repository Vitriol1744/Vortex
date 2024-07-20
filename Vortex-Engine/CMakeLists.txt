# 
#  Created by v1tr10l7 on 17.07.2024.
#  Copyright (c) 2024-2024, Szymon Zemke <v1tr10l7@proton.me>
# 
#  SPDX-License-Identifier: GPL-3
#
cmake_minimum_required (VERSION 3.8)
project (Vortex-Engine)

set (CMAKE_CXX_STANDARD 23)
set (CXX_STANDARD_REQUIRED ON)

find_package (Vulkan REQUIRED)

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

add_subdirectory (vendor/fmt)
add_subdirectory (vendor/glfw)
add_subdirectory (vendor/glm)
add_subdirectory (vendor/magic_enum)

if (CMAKE_BUILD_TYPE MATCHES Debug)
    add_definitions (-DVT_DEBUG)
elseif (CMAKE_BUILD_TYPE MATCHES Release)
    add_definitions (-DVT_RELEASE)
elseif (CMAKE_BUILD_TYPE MATCHES Dist)
    add_definitions (-DVT_DIST)
endif ()

add_definitions (-DVT_BUILD_DLL)
add_definitions (-DVT_ENABLE_ASSERTIONS)

set (INCLUDE_DIRS ${INCLUDE_DIRS} "${CMAKE_CURRENT_SOURCE_DIR}/src/Vortex" ${Vulkan_INCLUDE_DIRS} vendor/imgui)
set (LIBS ${Vulkan_LIBRARIES} magic_enum::magic_enum fmt::fmt-header-only glfw glm::glm-header-only)

set (SOURCES
    src/Vortex/Core/EventSystem.cpp
    src/Vortex/Core/Log/Log.cpp
    src/Vortex/Core/Log/Logger.cpp
    src/Vortex/Core/Memory/Memory.cpp
    src/Vortex/Engine/Application.cpp
    src/Vortex/Engine/EntryPoint.cpp
    src/Vortex/Graphics/API/RendererContext.cpp
    src/Vortex/Graphics/API/Vulkan/VulkanCommon.cpp
    src/Vortex/Graphics/API/Vulkan/VulkanContext.cpp
    src/Vortex/Graphics/API/Vulkan/VulkanDevice.cpp
    src/Vortex/Graphics/API/Vulkan/VulkanInstance.cpp
    src/Vortex/Graphics/API/Vulkan/VulkanSurface.cpp
    src/Vortex/Graphics/API/Vulkan/VulkanSwapChain.cpp
    src/Vortex/Graphics/Window/IWindow.cpp
)

if (UNIX)
    set (SOURCES ${SOURCES} src/Vortex/Graphics/Window/X11/X11Window.cpp)
endif ()

# ImGui
set (SOURCES ${SOURCES} 
    vendor/imgui/imgui.cpp 
    vendor/imgui/imgui_demo.cpp 
    vendor/imgui/imgui_draw.cpp
    vendor/imgui/imgui_tables.cpp
    vendor/imgui/imgui_widgets.cpp
    src/Vortex/Engine/imgui_impl_glfw.cpp
    src/Vortex/Engine/imgui_impl_vulkan.cpp)

add_library (${PROJECT_NAME} SHARED ${SOURCES})

target_precompile_headers(${PROJECT_NAME} PUBLIC src/vtpch.hpp)
target_compile_options(${PROJECT_NAME} PRIVATE
  $<$<CXX_COMPILER_ID:MSVC>:/W4 /WX>
  $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra -Wpedantic -Werror>
)
add_compile_options (-fvisibility=hidden)

target_include_directories(${PROJECT_NAME} PRIVATE ${INCLUDE_DIRS})
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries (${PROJECT_NAME} PUBLIC ${LIBS})
