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

# The Tracy client is always linked; VT_ENABLE_PROFILING (top-level
# CMakeLists) decides whether its macros do anything.
set(TRACY_ENABLE ${VT_ENABLE_PROFILING} CACHE BOOL "" FORCE)
set(TRACY_ON_DEMAND ${VT_ENABLE_PROFILING} CACHE BOOL "" FORCE)

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
endif ()

# --- Prism (C++20 named modules) ---
# Built from source: module BMIs must be compiled with flags compatible with
# their importers, so a prebuilt system Prism can't be used.
set(PRISM_TARGET "app" CACHE STRING "" FORCE)
set(PRISM_WERROR OFF CACHE BOOL "" FORCE)
set(PRISM_BUILD_TESTS OFF CACHE BOOL "" FORCE)
# Shared, so Vortex-Core, the other engine libraries and the executables all
# use one copy of Prism (its globals, type info, log sinks) instead of each
# linking its own.
if(NOT ANDROID)
    set(PRISM_SHARED ON CACHE BOOL "" FORCE)
endif()
add_subdirectory(vendor/Prism)
if(NOT ANDROID)
    # Next to the engine's own shared libraries.
    set_target_properties(prism PROPERTIES LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
endif()
# Vortex expects Prism's names in the global namespace. Named modules don't
# see the importer's macros, so this must be set where Prism itself is built.
target_compile_definitions(prism PUBLIC PRISM_USE_NAMESPACE=1)
# Prism builds with -fno-rtti/-fno-exceptions and exports them to consumers,
# but Vortex uses both (typeid, nlohmann::json exceptions), and clang refuses
# to import a BMI built with different RTTI/exception settings. Re-enable
# them for Prism and its importers; the last flag on the command line wins.
target_compile_options(prism PUBLIC -frtti -fexceptions)
# add_subdirectory(vendor/magic_enum)
add_subdirectory(vendor/shaderc)
add_subdirectory(vendor/SPIRV-Cross)
add_subdirectory(vendor/tracy)

option(VT_OPTIMIZE_THIRD_PARTY
  "Build hot third-party libraries (PhysX, ImGui) optimised even in Debug" ON)

# --- NVIDIA PhysX (CPU only, static) ---
# Consumed through its FetchContent-style entry point, which builds the SDK as
# a subproject (EXCLUDE_FROM_ALL) and gathers the libraries in `physx_lib`.
if (NOT ANDROID)
  set(PX_GENERATE_GPU_PROJECTS OFF CACHE BOOL "" FORCE)
  set(PX_GENERATE_GPU_PROJECTS_ONLY OFF CACHE BOOL "" FORCE)
  set(PX_GENERATE_STATIC_LIBRARIES ON CACHE BOOL "" FORCE)
  set(PX_BUILDSNIPPETS OFF CACHE BOOL "" FORCE)
  set(PX_BUILDPVDRUNTIME OFF CACHE BOOL "" FORCE)
  add_subdirectory(vendor/PhysX/physx)
  # PhysX builds with -Werror -Weverything, which newer compilers trip over;
  # target options come last on the command line, so these win.
  foreach(_px_lib IN ITEMS PhysX PhysXCommon PhysXFoundation PhysXExtensions
          PhysXPvdSDK PhysXCooking PhysXCharacterKinematic PhysXVehicle
          LowLevel LowLevelAABB LowLevelDynamics SceneQuery SimulationController
          PhysXTask FastXml)
    if (TARGET ${_px_lib})
      target_compile_options(${_px_lib} PRIVATE -w -Wno-error)
      if (VT_OPTIMIZE_THIRD_PARTY)
        # Unoptimised PhysX is ~10x slower; nobody steps through its solver.
        # Comes after the -O0 of the Debug config, so it wins.
        target_compile_options(${_px_lib} PRIVATE -O2 -g1)
      endif()
    endif()
  endforeach()
endif ()

# ImGui runs every frame and is never debugged; optimise it in Debug too.
if (VT_OPTIMIZE_THIRD_PARTY)
  foreach(_hot_lib IN ITEMS imgui ImGuizmo)
    if (TARGET ${_hot_lib})
      target_compile_options(${_hot_lib} PRIVATE -O2 -g1)
    endif()
  endforeach()
endif()

# --- Find External System Packages ---
find_package(Vulkan REQUIRED)
if (ANDROID)
  # find_package(AndroidNDK REQUIRED)
  # add_subdirectory(vendor/VulkanMemoryAllocator)
else ()
  add_subdirectory(vendor/VulkanMemoryAllocator)

  find_package(fmt REQUIRED)
  find_package(PkgConfig REQUIRED)
endif ()


