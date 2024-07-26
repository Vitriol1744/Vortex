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

add_subdirectory (vendor/fmt)
add_subdirectory (vendor/magic_enum)
add_subdirectory (vendor/glm)
add_subdirectory (vendor/glfw)

if (CMAKE_BUILD_TYPE MATCHES Debug)
    add_definitions (-DVT_DEBUG)
elseif (CMAKE_BUILD_TYPE MATCHES RelWithDebInfo)
    add_definitions (-DVT_RELEASE)
elseif (CMAKE_BUILD_TYPE MATCHES Release)
    add_definitions (-DVT_DIST)
endif ()

#add_definitions (-DVT_STATIC_LIBRARY)
add_definitions (-DVT_BUILD_DLL)
add_definitions (-DVT_ENABLE_ASSERTIONS)

set (INCLUDE_DIRS ${INCLUDE_DIRS} "${CMAKE_CURRENT_SOURCE_DIR}/src/Vortex")
set (LIBS ${Vulkan_LIBRARIES} fmt::fmt-header-only magic_enum::magic_enum glfw glm::glm-header-only)

set (SOURCES
    src/Vortex/Core/Events/EventSystem.cpp
    src/Vortex/Core/Log/Log.cpp
    src/Vortex/Core/Log/Logger.cpp
    src/Vortex/Engine/Application.cpp
    src/Vortex/Engine/EntryPoint.cpp
    src/Vortex/Graphics/API/RendererContext.cpp
    src/Vortex/Graphics/API/Vulkan/VulkanCommon.cpp
    src/Vortex/Graphics/API/Vulkan/VulkanContext.cpp
    src/Vortex/Graphics/API/Vulkan/VulkanInstance.cpp
    src/Vortex/Graphics/Window/Monitor.cpp
    src/Vortex/Graphics/Window/Window.cpp
)

if (UNIX)
    set (SOURCES ${SOURCES}
        src/Vortex/Graphics/Window/X11/X11Window.cpp
        src/Vortex/Graphics/Window/X11/X11Monitor.cpp)
endif ()

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
