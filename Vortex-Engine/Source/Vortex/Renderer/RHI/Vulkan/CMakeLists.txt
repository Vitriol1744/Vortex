#
# Created by v1tr10l7 on 10.07.2026. Copyright (c) 2024-2026, Szymon Zemke
# <v1tr10l7@proton.me>
#
# SPDX-License-Identifier: GPL-3
#
set (MODULE_NAME Vortex-Vulkan)
set (MODULE_TYPE SHARED)

#TODO(v1tr10l7): Conditional compilation for imgui layer, maybe add a flag to enable/disable it in the future
set (SOURCES
    VulkanAllocator.cpp
    VulkanCommon.cpp
    VulkanDevice.cpp
    VulkanFramebuffer.cpp
    VulkanGraphicsPipeline.cpp
    VulkanImGuiLayer.cpp
    VulkanImage.cpp
    VulkanIndexBuffer.cpp
    VulkanInstance.cpp
    VulkanMaterial.cpp
    VulkanRenderer.cpp
    VulkanShader.cpp
    VulkanSurface.cpp
    VulkanSwapChain.cpp
    VulkanTexture2D.cpp
    VulkanUniformBuffer.cpp
    VulkanVertexBuffer.cpp
    imgui_impl_vulkan.cpp
)

vt_add_module(${MODULE_NAME}
    SOURCES ${SOURCES}
    TYPE SHARED
    DEFINITIONS_PUBLIC
        SPIRV_CROSS_EXCEPTIONS_TO_ASSERTIONS=1
    DEFINITIONS_PRIVATE
        VULKAN_HPP_DISPATCH_LOADER_DYNAMIC=1
    PUBLIC_INCLUDES
        ${VENDOR_DIR}/vulkan
        ${VENDOR_DIR}/VulkanMemoryAllocator/include
        ${VENDOR_DIR}/imgui
    PUBLIC_LIBS
        Vortex-Modules
        Vortex-Core
        Vortex-RHI
        Vulkan::Vulkan
        vulkan
        Vulkan::Headers
        imgui
        ImGuizmo
        shaderc
        shaderc_util
        spirv-cross-reflect
        spirv-cross-glsl
        spirv-cross-hlsl
        spirv-cross-cpp
        SPIRV-Tools-shared
)
