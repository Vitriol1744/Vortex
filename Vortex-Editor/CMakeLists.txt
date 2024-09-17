#
# Created by v1tr10l7 on 07.09.2024. Copyright (c) 2024-2024, Szymon Zemke
# <v1tr10l7@proton.me>
#
# SPDX-License-Identifier: GPL-3
#
cmake_minimum_required(VERSION 3.27)
project(Vortex-Editor)

set(CMAKE_CXX_STANDARD 23)
set(CXX_STANDARD_REQUIRED ON)

add_definitions(-DVT_ENABLE_ASSERTIONS)

set(SOURCES src/EditorApplication.cpp src/EditorLayer.cpp
            src/Panels/SceneHierarchyPanel.cpp)
add_executable(${PROJECT_NAME} ${SOURCES})
target_include_directories(${PROJECT_NAME} PRIVATE src)
target_link_libraries(${PROJECT_NAME} PUBLIC Vortex-Engine)
