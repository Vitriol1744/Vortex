#
# Created by v1tr10l7 on 10.07.2026. Copyright (c) 2024-2026, Szymon Zemke
# <v1tr10l7@proton.me>
#
# SPDX-License-Identifier: GPL-3
#
set (MODULE_NAME Vortex-RHI)
set (MODULE_TYPE STATIC)

set (SOURCES
    Framebuffer.cpp
    GraphicsPipeline.cpp
    ImGuiLayer.cpp
    IndexBuffer.cpp
    Material.cpp
    RHI.cpp
    Shader.cpp
    SwapChain.cpp
    Texture2D.cpp
    UniformBuffer.cpp
    VertexBuffer.cpp
)

vt_add_module(Vortex-RHI
    SOURCES ${SOURCES}
    TYPE STATIC
    DEFINITIONS_PUBLIC
        SPIRV_CROSS_EXCEPTIONS_TO_ASSERTIONS=1
    DEFINITIONS_PRIVATE
        VULKAN_HPP_DISPATCH_LOADER_DYNAMIC=1
    PUBLIC_INCLUDES
        ${VENDOR_DIR}/vulkan
        ${VENDOR_DIR}/imgui
        ${VENDOR_DIR}/VulkanMemoryAllocator/include
    PUBLIC_LIBS
        Vortex-Modules
        Vortex-Core
        Vortex-Window
        Vulkan::Vulkan
        Vulkan::Headers
        imgui
        ImGuizmo
)
add_subdirectory(Vulkan)
