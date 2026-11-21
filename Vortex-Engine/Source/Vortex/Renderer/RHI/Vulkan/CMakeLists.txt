#
# Created by v1tr10l7 on 18.06.2026. Copyright (c) 2024-2026, Szymon Zemke
# <v1tr10l7@proton.me>
#
# SPDX-License-Identifier: GPL-3
#

# The Vulkan backend of the RHI. Its headers are private: the only public
# interface is the Vortex.RHI.Vulkan module (RegisterBackend).
vt_add_module(Vortex-Vulkan
    SOURCES
        VulkanCommandList.cpp
        VulkanCommon.cpp
        VulkanDevice.cpp
        VulkanInstance.cpp
        VulkanPipeline.cpp
        VulkanResources.cpp
        VulkanShader.cpp
        VulkanSurface.cpp
        VulkanSwapChain.cpp
    TYPE SHARED
    DEFINITIONS_PRIVATE
        VULKAN_HPP_DISPATCH_LOADER_DYNAMIC=1
    PRIVATE_INCLUDES
        ${VENDOR_DIR}/vulkan
        ${VENDOR_DIR}/VulkanMemoryAllocator/include
    PUBLIC_LIBS
        Vortex-Modules
        Vortex-Core
        Vortex-RHI
        Vortex-Window
    PRIVATE_LIBS
        Vulkan::Vulkan
        Vulkan::Headers
        shaderc
        glslang
        SPIRV
        glslang-default-resource-limits
        spirv-cross-core
)
target_sources(Vortex-Vulkan PUBLIC FILE_SET CXX_MODULES FILES
    VulkanBackend.ixx
)

# Desktop builds link VMA's prebuilt implementation; Android compiles it in
# VulkanDevice.cpp (VMA_IMPLEMENTATION).
if(NOT ANDROID)
  target_link_libraries(Vortex-Vulkan PRIVATE GPUOpen::VulkanMemoryAllocator)
endif()

# GPU zones (Tracy's Vulkan timestamps) when profiling is built in.
if(TARGET Tracy::TracyClient)
  target_link_libraries(Vortex-Vulkan PRIVATE Tracy::TracyClient)
endif()
