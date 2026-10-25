#
# Created by v1tr10l7 on 16.09.2026. Copyright (c) 2024-2026, Szymon Zemke
# <v1tr10l7@proton.me>
#
# SPDX-License-Identifier: GPL-3
#
add_subdirectory(RHI)

set (MODULE_NAME Vortex-Renderer)
set (MODULE_TYPE STATIC)

set (SOURCES
  Image.cpp
  Mesh.cpp
  Renderer.cpp
  Renderer2D.cpp
  SceneRenderer.cpp
  ShaderLibrary.cpp
  TextureAsset.cpp
  ImGui/ImGui.cpp
  RHI/Vulkan/imgui_impl_vulkan.cpp
)

vt_add_module(${MODULE_NAME}
    SOURCES ${SOURCES}
    TYPE ${MODULE_TYPE}
    PUBLIC_LIBS
      Vortex-Modules
      Vortex-Asset
      Vortex-RHI
      Vortex-Window
      assimp
      imgui
      ImGuizmo
)
