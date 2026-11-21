#
# Created by v1tr10l7 on 17.09.2026. Copyright (c) 2024-2026, Szymon Zemke
# <v1tr10l7@proton.me>
#
# SPDX-License-Identifier: GPL-3
#
set (MODULE_NAME Vortex-Scene)
set (MODULE_TYPE STATIC)

set (SOURCES
  Actor.cpp
  Camera.cpp
  FirstPerson.cpp
  Scene.cpp
  SceneRenderer.cpp
)

vt_add_module(${MODULE_NAME}
    SOURCES ${SOURCES}
    TYPE ${MODULE_TYPE}
    PUBLIC_LIBS
      Vortex-Modules
      Vortex-Filesystem
      Vortex-Physics
      Vortex-Renderer
      Vortex-Script
      nlohmann_json::nlohmann_json
    PUBLIC_INCLUDES
      ${VENDOR_DIR}/entt/single_include
    DEFINITIONS_PUBLIC
      GLM_FORCE_RADIANS
      GLM_FORCE_DEPTH_ZERO_TO_ONE
      GLM_ENABLE_EXPERIMENTAL
)
