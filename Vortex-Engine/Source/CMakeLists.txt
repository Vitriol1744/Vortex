#
# Created by v1tr10l7 on 18.06.2026. Copyright (c) 2024-2026, Szymon Zemke
# <v1tr10l7@proton.me>
#
# SPDX-License-Identifier: GPL-3
#
add_library(Vortex-Modules INTERFACE)



if(TARGET Vortex-Wayland-Gen)
    add_dependencies(Vortex-Modules Vortex-Wayland-Gen)
endif()
target_compile_options(Vortex-Modules INTERFACE
    $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-fvisibility=hidden $<$<COMPILE_LANGUAGE:CXX>:-fvisibility-inlines-hidden>>)

target_include_directories(Vortex-Modules INTERFACE 
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../vendor/stb/include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../vendor/vulkan>
    $<BUILD_INTERFACE:${VORTEX_WAYLAND_GEN_DIR}> 
)
target_link_libraries(Vortex-Modules INTERFACE
    Vortex-Config
    $<BUILD_INTERFACE:imgui>
)
#TODO(v1tr10l7): fix vortex with assertions disabled
target_compile_options (Vortex-Modules INTERFACE -DPRISM_USE_NAMESPACE=1 -DVT_ENABLE_ASSERTIONS)
include(GenerateExportHeader)

set (MODULE_TYPE STATIC)
add_subdirectory (Vortex/Asset)
add_subdirectory (Vortex/Core)
add_subdirectory (Vortex/EntryPoint)
add_subdirectory (Vortex/Filesystem)
add_subdirectory (Vortex/Project)
add_subdirectory (Vortex/Renderer)
add_subdirectory (Vortex/Scene)
add_subdirectory (Vortex/Script)
add_subdirectory (Vortex/Window)
