#
# Created by v1tr10l7 on 07.09.2024. Copyright (c) 2024-2024, Szymon Zemke
# <v1tr10l7@proton.me>
#
# SPDX-License-Identifier: GPL-3
#
cmake_minimum_required(VERSION 3.28)
project(Nexus)

# Nexus imports Prism's named modules. The C++ standard and the engine-wide
# definitions come from the top-level CMakeLists and Vortex-Config.
set(CMAKE_CXX_SCAN_FOR_MODULES ON)

set(SOURCES
    Source/Nexus/Commands.cpp
    Source/Nexus/EditorApplication.cpp
    Source/Nexus/EditorCamera.cpp
    Source/Nexus/EditorCommands.cpp
    Source/Nexus/EditorContext.cpp
    Source/Nexus/EditorLayer.cpp
    Source/Nexus/EditorPreferences.cpp
    Source/Nexus/EditorProject.cpp
    Source/Nexus/EditorResources.cpp
    Source/Nexus/SceneController.cpp
    Source/Nexus/ScriptBuildService.cpp
    Source/Nexus/TestScenes.cpp
    Source/Nexus/UI/EditorWindows.cpp
    Source/Nexus/UI/Inspector.cpp
    Source/Nexus/UI/MainMenuBar.cpp
    Source/Nexus/UI/Toolbar.cpp
    Source/Nexus/UI/WindowManager.cpp
    Source/Nexus/UI/Modals/NewProjectModal.cpp
    Source/Nexus/UI/Overlays/SettingsOverlay.cpp
    Source/Nexus/UI/Windows/ConsoleWindow.cpp
    Source/Nexus/UI/Windows/ContentBrowserWindow.cpp
    Source/Nexus/UI/Windows/SceneHierarchyWindow.cpp
    Source/Nexus/UI/Windows/SceneViewportWindow.cpp
    Source/Nexus/UI/Windows/ToolWindows.cpp
)
add_executable(${PROJECT_NAME} ${SOURCES})
target_sources(${PROJECT_NAME} PRIVATE FILE_SET CXX_MODULES FILES
    Source/Nexus/Commands.ixx
    Source/Nexus/ProjectTemplates.ixx
    Source/Nexus/TestScenes.ixx
    Source/Nexus/UI/EditorWindow.ixx
    Source/Nexus/UI/Inspector.ixx
)
target_include_directories(${PROJECT_NAME} PRIVATE Source)
# Vortex-Engine brings every engine module (and Prism, through Vortex-Core).
target_link_libraries(${PROJECT_NAME} PRIVATE Vortex-Engine)
target_compile_options(${PROJECT_NAME} PRIVATE -Wno-nontrivial-memcall)
