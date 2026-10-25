#
# Created by v1tr10l7 on 07.09.2024. Copyright (c) 2024-2024, Szymon Zemke
# <v1tr10l7@proton.me>
#
# SPDX-License-Identifier: GPL-3
#
cmake_minimum_required(VERSION 3.27)
project(Nexus)

set(CMAKE_CXX_STANDARD 23)
set(CXX_STANDARD_REQUIRED ON)

add_definitions(-DVT_ENABLE_ASSERTIONS -DPRISM_USE_NAMESPACE=1)

set(SOURCES 
    Source/Nexus/EditorApplication.cpp 
    Source/Nexus/EditorCamera.cpp 
    Source/Nexus/EditorLayer.cpp
    Source/Nexus/EditorPreferences.cpp
    Source/Nexus/EditorProject.cpp
    Source/Nexus/EditorResources.cpp
    Source/Nexus/UI/Modals/NewProjectModal.cpp
    Source/Nexus/UI/Panels/ConsolePanel.cpp
    Source/Nexus/UI/Panels/ContentBrowserPanel.cpp
    Source/Nexus/UI/Panels/SceneHierarchyPanel.cpp
)
add_executable(${PROJECT_NAME} ${SOURCES})
target_include_directories(${PROJECT_NAME} PRIVATE Source)
target_link_libraries(${PROJECT_NAME} PUBLIC Vortex-Engine Vortex-Core Vortex-EntryPoint Vortex-Filesystem Vortex-Project Vortex-RHI Vortex-Vulkan Vortex-Script Vortex-Asset $<LINK_LIBRARY:WHOLE_ARCHIVE,prism>)
target_compile_options(${PROJECT_NAME} PRIVATE -Wno-nontrivial-memcall)
