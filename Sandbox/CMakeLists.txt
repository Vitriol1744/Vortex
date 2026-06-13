#
# Created by v1tr10l7 on 17.07.2024. Copyright (c) 2024-2024, Szymon Zemke
# <v1tr10l7@proton.me>
#
# SPDX-License-Identifier: GPL-3
#
cmake_minimum_required(VERSION 3.27)
project(SandboxApp)

set(CMAKE_CXX_STANDARD 23)
set(CXX_STANDARD_REQUIRED ON)

add_definitions(-DVT_ENABLE_ASSERTIONS -DPRISM_USE_NAMESPACE=1)

set(SOURCES Source/Sandbox.cpp Source/SandboxLayer2D.cpp Source/ImGuiPanels.cpp)
add_executable(${PROJECT_NAME} ${SOURCES})
target_link_libraries(${PROJECT_NAME} PUBLIC Vortex-Engine)
target_compile_options(${PROJECT_NAME} PRIVATE -Wno-nontrivial-memcall -Wno-uninitialized-const-pointer)
