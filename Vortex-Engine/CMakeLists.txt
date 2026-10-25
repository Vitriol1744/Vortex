#
# Created by v1tr10l7 on 17.07.2024. Copyright (c) 2024-2024, Szymon Zemke
# <v1tr10l7@proton.me>
#
# SPDX-License-Identifier: GPL-3
#
cmake_minimum_required(VERSION 3.25)
project(Vortex-Engine VERSION 0.1)

include(cmake/Dependencies.cmake)
include(cmake/Module.cmake)

option(VORTEX_BUILD_SHARED_LIBS "Build using shared libraries" OFF)
option(VT_BUILD_WAYLAND "Build vortex with wayland support" ON)

# ------------------------------------------------------------------------------
# Build Environment Summary
# ------------------------------------------------------------------------------
message(STATUS "--------------------------------------------------")
message(STATUS " Configuring ${PROJECT_NAME} v${PROJECT_VERSION}")
message(STATUS " Target System: ${CMAKE_SYSTEM_NAME} (${CMAKE_SYSTEM_PROCESSOR})")
message(STATUS " Build Type:    ${CMAKE_BUILD_TYPE}")
message(STATUS " C++ Standard:  C++${CMAKE_CXX_STANDARD}")
message(STATUS "--------------------------------------------------")

# ------------------------------------------------------------------------------
# Defines & Core Settings
# ------------------------------------------------------------------------------
set (VORTEX_ENGINE_SOURCE_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/Source")
set (VENDOR_DIR ${CMAKE_CURRENT_SOURCE_DIR}/vendor)
set (VORTEX_WAYLAND_GEN_DIR "${CMAKE_CURRENT_BINARY_DIR}" CACHE INTERNAL "")
set (WAYLAND_GENERATED_FILES "" CACHE INTERNAL "List of generated wayland files")

if(ANDROID)
  list (APPEND PUBLIC_DEFINES VT_PLATFORM_ANDROID)
  message(STATUS "Platform: Android build enabled")
elseif (VT_BUILD_WAYLAND)
  message(STATUS "Platform: Wayland build enabled")
endif()

add_subdirectory(Source)

set(SOURCES
    Source/Vortex/Engine/Application.cpp
    Source/Vortex/Engine/Engine.cpp
    Source/Vortex/Engine/RuntimeAssetManager.cpp
    Source/Vortex/Utility/ImageLoader.cpp
    Source/Vortex/Utility/Image.cpp
    Source/Vortex/Utility/ZLib.cpp
)

if (ANDROID)
  set (SOURCES ${SOURCES} 
    ${VENDOR_DIR}/android_native_app_glue/android_native_app_glue.c
  )
else ()
  find_package (PkgConfig REQUIRED)
endif ()

# ------------------------------------------------------------------------------
# Target Definition
# ------------------------------------------------------------------------------
if(VORTEX_BUILD_SHARED_LIBS OR ANDROID) 
  list(APPEND PUBLIC_DEFINES VT_BUILD_DLL)
  add_library(${PROJECT_NAME} SHARED ${SOURCES})
  message(STATUS "Target Output: Shared Library (.so / .dll)")
else()
  add_library(${PROJECT_NAME} STATIC ${SOURCES})
  list(APPEND PUBLIC_DEFINES VT_STATIC_LIBRARY)
  message(STATUS "Target Output: Static Library (.a / .lib)")
endif()

# ------------------------------------------------------------------------------
# Includes
# ------------------------------------------------------------------------------
target_include_directories(${PROJECT_NAME}
  PRIVATE
      Source/Vortex
      vendor/assimp/contrib/zlib/contrib
      vendor/assimp/contrib/unzip
      ${CMAKE_CURRENT_BINARY_DIR}
  PUBLIC
      Source
      vendor/stb/include
      vendor/VulkanMemoryAllocator/include
      vendor/vulkan
)

if(ANDROID)
  target_include_directories(${PROJECT_NAME}
      PRIVATE
          vendor/vulkan
          vendor/android_native_app_glue
  )
endif()

# ------------------------------------------------------------------------------
# Dependencies & Linking
# ------------------------------------------------------------------------------
set(PUBLIC_LIBS
    Vortex-Asset
    Vortex-Core
    Vortex-EntryPoint
    Vortex-Filesystem
    Vortex-Project
    Vortex-Renderer
    Vortex-Scene
    Vortex-Script
    Vortex-Vulkan
    Vortex-Window
    Vulkan::Vulkan
    Vulkan::Headers
)

if(ANDROID)
  list(APPEND PUBLIC_LIBS vulkan Prism log)
  target_link_libraries(${PROJECT_NAME} PUBLIC ${PUBLIC_LIBS} $<LINK_LIBRARY:WHOLE_ARCHIVE,Prism>)
else()
  list(APPEND PUBLIC_LIBS GPUOpen::VulkanMemoryAllocator fmt)
  target_link_libraries(${PROJECT_NAME} PUBLIC ${PUBLIC_LIBS} $<LINK_LIBRARY:WHOLE_ARCHIVE,prism>)
endif()

if(NOT CMAKE_BUILD_TYPE MATCHES Release)
  target_link_libraries(${PROJECT_NAME} PUBLIC Tracy::TracyClient)
  message(STATUS "Profiler: Tracy profiling integration active")
endif()

# ------------------------------------------------------------------------------
# Compiler Options & Precompiled Headers
# ------------------------------------------------------------------------------
target_precompile_headers(${PROJECT_NAME} PUBLIC
    "$<$<COMPILE_LANGUAGE:CXX>:${CMAKE_CURRENT_SOURCE_DIR}/Source/vtpch.hpp>"
)

target_compile_options(${PROJECT_NAME} PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>:
        /W4>
    $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:
        -Wall
        -Wextra
        -Werror
        -Wno-nullability-extension
        -Wno-nontrivial-memcall
        -Wno-uninitialized-const-pointer
        -Wno-nullability-completeness
        -Wno-unknown-warning-option
        -Wno-unused-variable
        -Wno-unused-parameter
        -Wno-unused-function
        -Wno-missing-field-initializers
        -Wno-nested-anon-types
        -Wno-builtin-macro-redefined
        -Wno-unused-command-line-argument
        -Wno-deprecated-literal-operator
        -Wno-variadic-macros>
)

if(NOT ANDROID AND (CMAKE_CXX_COMPILER_ID MATCHES "Clang" OR CMAKE_CXX_COMPILER_ID MATCHES "GCC"))
  target_compile_options(${PROJECT_NAME} PUBLIC $<$<COMPILE_LANGUAGE:CXX>:-stdlib=libstdc++>)
endif()

# Sanitizers
if(NOT ANDROID AND (CMAKE_BUILD_TYPE MATCHES Debug OR CMAKE_BUILD_TYPE MATCHES RelWithDebInfo))
  target_compile_options(${PROJECT_NAME} PUBLIC -fsanitize=address)
  target_link_options(${PROJECT_NAME} PUBLIC -fsanitize=address)
  message(STATUS "Sanitizers: AddressSanitizer (ASan) enabled")
endif()

target_compile_definitions(${PROJECT_NAME} PUBLIC ${PUBLIC_DEFINES})
message(STATUS "Configuration complete for ${PROJECT_NAME}.")
