#
# Created by v1tr10l7 on 18.11.2026. Copyright (c) 2024-2026, Szymon Zemke
# <v1tr10l7@proton.me>
#
# SPDX-License-Identifier: GPL-3
#
# Rigid-body physics on NVIDIA PhysX (Vortex.Physics). PhysX stays private:
# its headers only appear in the implementation files.
set (MODULE_NAME Vortex-Physics)
set (MODULE_TYPE STATIC)

set (SOURCES
  PhysicsEngine.cpp
  PhysicsScene.cpp
)

vt_add_module(${MODULE_NAME}
    SOURCES ${SOURCES}
    TYPE ${MODULE_TYPE}
    PUBLIC_LIBS
      Vortex-Modules
      Vortex-Core
    PRIVATE_LIBS
      # The modules Scene's headers import (Vortex.Renderer...).
      Vortex-Renderer
      physx_lib
)
target_sources(${MODULE_NAME} PUBLIC FILE_SET CXX_MODULES FILES
    PhysicsEngine.ixx
)

# Physics reads scenes through Vortex-Scene's headers only, so it takes their
# usage requirements without linking the target: Scene and Script link
# Physics, and CMake leaves targets that form a link cycle out of each other's
# module lookup ("module 'Vortex.Physics' not found" in Scene and Script).
target_include_directories(${MODULE_NAME} PRIVATE
    $<TARGET_PROPERTY:Vortex-Scene,INTERFACE_INCLUDE_DIRECTORIES>)
target_compile_definitions(${MODULE_NAME} PRIVATE
    $<TARGET_PROPERTY:Vortex-Scene,INTERFACE_COMPILE_DEFINITIONS>)
