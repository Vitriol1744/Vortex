#*
#* Created by v1tr10l7 on 10.06.2026.
#* Copyright (c) 2024-2026, Szymon Zemke <v1tr10l7@proton.me>
#*
#* SPDX-License-Identifier: GPL-3
#*/

# --- Third-Party Configuration Options ---
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

set(SHADERC_SKIP_TESTS ON CACHE BOOL "" FORCE)
set(SHADERC_SKIP_EXAMPLES ON CACHE BOOL "" FORCE)

set(SPIRV_SKIP_EXECUTABLES ON CACHE BOOL "" FORCE)
set(ENABLE_GLSLANG_BINARIES OFF CACHE BOOL "" FORCE)

set(SPIRV_CROSS_ENABLE_TESTS OFF CACHE BOOL "" FORCE)
set(SPIRV_CROSS_ENABLE_C_API OFF CACHE BOOL "" FORCE)
set(SPIRV_CROSS_SKIP_INSTALL ON CACHE BOOL "" FORCE)
set(SPIRV_CROSS_CLI OFF CACHE BOOL "" FORCE)
# Force all downstream static targets (like SPIRV-Cross) to use position-independent code
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# If SPIRV-Cross provides explicit options in its CMake layout, explicitly enforce them:
set(SPIRV_CROSS_STATIC ON CACHE BOOL "" FORCE)
set(SPIRV_CROSS_SHARED OFF CACHE BOOL "" FORCE)

set(ASSIMP_BUILD_MINIZIP ON CACHE BOOL "Build Assimp with internal minizip" FORCE)
set(ASSIMP_INSTALL OFF CACHE BOOL "Disable Assimp installation rules" FORCE)
set(ASSIMP_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(ASSIMP_WARNINGS_AS_ERRORS OFF CACHE BOOL "" FORCE)

if(NOT CMAKE_BUILD_TYPE MATCHES "Release")
    set(TRACY_ENABLE ON CACHE BOOL "" FORCE)
    set(TRACY_ON_DEMAND ON CACHE BOOL "" FORCE)
else()
    set(TRACY_ENABLE OFF CACHE BOOL "" FORCE)
    set(TRACY_ON_DEMAND OFF CACHE BOOL "" FORCE)
endif()

# --- Add Vendor Subdirectories ---
add_subdirectory(vendor/assimp)
add_subdirectory(vendor/Coral/cmake)
add_subdirectory(vendor/entt)
add_subdirectory(vendor/glm)
add_subdirectory(vendor/imgui)
add_subdirectory(vendor/ImGuizmo)
add_subdirectory(vendor/json)
if (NOT ANDROID)
  add_subdirectory(vendor/glfw)
  add_subdirectory(vendor/nativefiledialog-extended)
else ()
  add_subdirectory(vendor/Prism)
endif ()
# add_subdirectory(vendor/magic_enum)
add_subdirectory(vendor/shaderc)
add_subdirectory(vendor/SPIRV-Cross)
add_subdirectory(vendor/tracy)

# --- Find External System Packages ---
find_package(Vulkan REQUIRED)
if (ANDROID)
  # find_package(AndroidNDK REQUIRED)
  # add_subdirectory(vendor/VulkanMemoryAllocator)
else ()
  add_subdirectory(vendor/VulkanMemoryAllocator)

  find_package(fmt REQUIRED)
  find_package(PkgConfig REQUIRED)
  pkg_check_modules(PRISM REQUIRED IMPORTED_TARGET prism)
endif ()


