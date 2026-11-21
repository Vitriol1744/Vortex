#
# Created by v1tr10l7 on 17.07.2024. Copyright (c) 2024-2024, Szymon Zemke
# <v1tr10l7@proton.me>
#
# SPDX-License-Identifier: GPL-3
#
# 3.28: C++20 named modules without the experimental gate.
cmake_minimum_required(VERSION 3.28)
project(Vortex)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
# Executables go to the source tree's bin/, which Nexus and Horizon run from
# (assets/ is next to it). Debug keeps bin/; other configurations get
# bin/<Config>/ so a release build doesn't overwrite the debug one.
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY
    "${CMAKE_CURRENT_SOURCE_DIR}/bin$<$<NOT:$<CONFIG:Debug>>:/$<CONFIG>>")

# Enable Hot Reload for MSVC compilers if supported.
if(POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT
      "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>"
  )
endif()

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Choose the type of build." FORCE)
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "RelWithDebInfo" "MinSizeRel")
endif()


# ------------------------------------------------------------------------------
# Options
# ------------------------------------------------------------------------------
# Debug and RelWithDebInfo builds are sanitized unless this is turned off.
option(VT_ENABLE_ASAN "Build Debug/RelWithDebInfo with AddressSanitizer" ON)
# Tracy profiling; off by default for Release and MinSizeRel.
if(CMAKE_BUILD_TYPE STREQUAL "Release" OR CMAKE_BUILD_TYPE STREQUAL "MinSizeRel")
    set(VT_PROFILING_DEFAULT OFF)
else()
    set(VT_PROFILING_DEFAULT ON)
endif()
option(VT_ENABLE_PROFILING "Build with the Tracy profiler client" ${VT_PROFILING_DEFAULT})

# ------------------------------------------------------------------------------
# Global Platform Detection
# ------------------------------------------------------------------------------
if(WIN32)
    set(VT_PLATFORM_DEFINE VT_PLATFORM_WINDOWS)
elseif(ANDROID)
    set(VT_PLATFORM_DEFINE VT_PLATFORM_ANDROID)
elseif(APPLE)
    set(VT_PLATFORM_DEFINE VT_PLATFORM_MACOS)
elseif(UNIX)
    set(VT_PLATFORM_DEFINE VT_PLATFORM_LINUX)
endif()

# Global Interface Target for Engine-Wide Configuration. Every Vortex target
# (and Nexus/Horizon, through them) gets these; PRISM_USE_NAMESPACE comes from
# the prism target itself.
add_library(Vortex-Config INTERFACE)
target_compile_definitions(Vortex-Config INTERFACE
    ${VT_PLATFORM_DEFINE}
    $<$<CONFIG:Debug>:VT_DEBUG>
    $<$<CONFIG:RelWithDebInfo>:VT_RELEASE>
    $<$<CONFIG:Release,MinSizeRel>:VT_DIST>
    # TODO(v1tr10l7): Vortex doesn't build with assertions disabled yet, so
    # they're on in every configuration for now.
    VT_ENABLE_ASSERTIONS
    GLM_FORCE_RADIANS
    GLM_FORCE_DEPTH_ZERO_TO_ONE
    GLM_ENABLE_EXPERIMENTAL
)

add_subdirectory(Vortex-Engine)
if (NOT ANDROID)
  add_subdirectory(Nexus)
  add_subdirectory(Horizon)
  # add_subdirectory(Sandbox)
endif ()
