#
# Created by v1tr10l7 on 18.11.2026. Copyright (c) 2024-2026, Szymon Zemke
# <v1tr10l7@proton.me>
#
# SPDX-License-Identifier: GPL-3
#
# Horizon: the Vortex launcher. Picks or creates a project, installs Nexus
# releases from a manifest, and starts Nexus with `--project <file.vproj>`.
#
cmake_minimum_required(VERSION 3.28)
project(Horizon)

set(CMAKE_CXX_SCAN_FOR_MODULES ON)

find_package(CURL REQUIRED)

add_executable(${PROJECT_NAME}
    Source/Horizon/HorizonApplication.cpp
    Source/Horizon/LauncherLayer.cpp
    Source/Horizon/Installs.cpp
    Source/Horizon/Platform.cpp
    Source/Horizon/Projects.cpp
)
target_sources(${PROJECT_NAME} PRIVATE FILE_SET CXX_MODULES FILES
    Source/Horizon/Cli.ixx
    Source/Horizon/Http.ixx
    Source/Horizon/Json.ixx
    Source/Horizon/Installs.ixx
    Source/Horizon/Manifest.ixx
    Source/Horizon/Platform.ixx
    Source/Horizon/Projects.ixx
    Source/Horizon/Settings.ixx
    Source/Horizon/Sha256.ixx
    Source/Horizon/Templates.ixx
    Source/Horizon/Widgets.ixx
)
target_include_directories(${PROJECT_NAME} PRIVATE Source)
# Only the window, renderer and ImGui are used at runtime; the rest of the
# engine is linked because Application lives in the static Vortex-Engine.
target_link_libraries(${PROJECT_NAME} PRIVATE
    Vortex-Engine
    nlohmann_json::nlohmann_json
    CURL::libcurl
)
target_compile_options(${PROJECT_NAME} PRIVATE -Wno-nontrivial-memcall)
