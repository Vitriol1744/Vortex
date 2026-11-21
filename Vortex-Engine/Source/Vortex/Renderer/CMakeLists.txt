#
# Created by v1tr10l7 on 18.06.2026. Copyright (c) 2024-2026, Szymon Zemke
# <v1tr10l7@proton.me>
#
# SPDX-License-Identifier: GPL-3
#
add_subdirectory(RHI)

# The rendering frontend: device ownership and frames (Vortex.Renderer),
# materials, meshes, textures, the scene/2D renderers and ImGui on the RHI.
# Shared: it owns the device and per-process renderer state.
vt_add_module(Vortex-Renderer
    SOURCES
        Animation.cpp
        Environment.cpp
        Image.cpp
        Material.cpp
        Mesh.cpp
        Renderer.cpp
        Renderer2D.cpp
        RenderGraph.cpp
        RenderTarget.cpp
        ShaderLibrary.cpp
        TextureAsset.cpp
        ImGui/ImGui.cpp
        ImGui/ImGuiLayer.cpp
        ImGui/ImGuiVortexBackend.cpp
    TYPE SHARED
    PUBLIC_LIBS
      Vortex-Modules
      Vortex-Asset
      Vortex-RHI
      Vortex-Window
      assimp
      imgui
      ImGuizmo
    PRIVATE_LIBS
      Vortex-Vulkan
)
target_sources(Vortex-Renderer PUBLIC FILE_SET CXX_MODULES FILES
    Environment.ixx
    Material.ixx
    Renderer.ixx
    RenderGraph.ixx
    RenderTarget.ixx
)
if(TARGET Tracy::TracyClient)
  target_link_libraries(Vortex-Renderer PUBLIC Tracy::TracyClient)
endif()
