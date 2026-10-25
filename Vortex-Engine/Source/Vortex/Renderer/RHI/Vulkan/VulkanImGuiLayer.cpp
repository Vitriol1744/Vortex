/*
 * Created by v1tr10l7 on 28.07.2024.
 * Copyright (c) 2024-2024, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#include <Vortex/Renderer/RHI/Vulkan/VulkanImGuiLayer.hpp>
#include <Vortex/Renderer/RHI/Vulkan/VulkanRenderer.hpp>
#include <Vortex/Renderer/RHI/Vulkan/VulkanSurface.hpp>
#include <Vortex/Renderer/RHI/Vulkan/imgui_impl_vulkan.h>

#include <vulkan/vulkan_core.h>

namespace Vortex
{
    ImGui_ImplVortex_Data* GetBackendData();
    inline static ImGuiViewportData* GetViewportData(ImGuiViewport* viewport)
    {
        return reinterpret_cast<ImGuiViewportData*>(viewport->PlatformUserData);
    }

    void VulkanImGuiLayer::OnUpdate(Timestep) {}
    void VulkanImGuiLayer::OnRender() {}
    void VulkanImGuiLayer::OnImGuiRender() {}

    void VulkanImGuiLayer::OnInitializeRendererInterface()
    {
        auto&     window        = m_MainWindow;
        auto      swapChain     = window.SwapChain().As<VulkanSwapChain>();
        vk::Queue graphicsQueue = VulkanRenderer::Device().GraphicsQueue();

        auto      poolSizes     = ToArray<vk::DescriptorPoolSize>(
            {{vk::DescriptorType::eSampler, 1000},
                      {vk::DescriptorType::eCombinedImageSampler, 1000},
                      {vk::DescriptorType::eSampledImage, 1000},
                      {vk::DescriptorType::eStorageImage, 1000},
                      {vk::DescriptorType::eUniformTexelBuffer, 1000},
                      {vk::DescriptorType::eStorageTexelBuffer, 1000},
                      {vk::DescriptorType::eUniformBuffer, 1000},
                      {vk::DescriptorType::eStorageBuffer, 1000},
                      {vk::DescriptorType::eUniformBufferDynamic, 1000},
                      {vk::DescriptorType::eStorageBufferDynamic, 1000},
                      {vk::DescriptorType::eInputAttachment, 1000}});

        vk::DescriptorPoolCreateInfo poolInfo = {};
        poolInfo.sType   = vk::StructureType::eDescriptorPoolCreateInfo;
        poolInfo.flags   = vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet;
        poolInfo.maxSets = 1000;
        poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.Size());
        poolInfo.pPoolSizes    = poolSizes.Raw();

        vk::Device device      = VulkanRenderer::Device();
        VkCall(device.createDescriptorPool(&poolInfo, VK_NULL_HANDLE,
                                           &m_DescriptorPool));

        auto& io = ImGui::GetIO();
        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        {
            ImGuiPlatformIO& io = ImGui::GetPlatformIO();
            io.Platform_CreateVkSurface  = CreateVulkanSurface;
        }
        auto& indices = VulkanRenderer::PhysicalDevice().QueueFamilyIndices();

        ImGui_ImplVulkan_InitInfo initInfo = {};
        initInfo.Instance = vk::Instance(VulkanRenderer::VulkanInstance());
        initInfo.PhysicalDevice
            = vk::PhysicalDevice(VulkanRenderer::PhysicalDevice());
        initInfo.Device                      = device;
        initInfo.QueueFamily                 = indices.Graphics.Value();
        initInfo.Queue                       = graphicsQueue;
        initInfo.PipelineCache               = VK_NULL_HANDLE;
        initInfo.DescriptorPool              = m_DescriptorPool;
        initInfo.PipelineInfoMain.RenderPass = swapChain->RenderPass();
        initInfo.PipelineInfoMain.Subpass    = 0;
        u32 frameCount         = static_cast<u32>(swapChain->ImageCount());
        initInfo.MinImageCount = frameCount;
        initInfo.ImageCount    = frameCount;
        initInfo.PipelineInfoMain.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
        initInfo.Allocator                    = VK_NULL_HANDLE;
        initInfo.CheckVkResultFn
            = [](VkResult result) -> void { VkCall(vk::Result(result)); };
        ImGui_ImplVulkan_Init(&initInfo);

        vk::CommandPoolCreateInfo commandPoolInfo{};
        commandPoolInfo.sType = vk::StructureType::eCommandPoolCreateInfo;
        commandPoolInfo.pNext = VK_NULL_HANDLE;
        commandPoolInfo.flags
            = vk::CommandPoolCreateFlagBits::eResetCommandBuffer;
        commandPoolInfo.queueFamilyIndex = VulkanRenderer::PhysicalDevice()
                                               .QueueFamilyIndices()
                                               .Graphics.Value();
        VkCall(device.createCommandPool(&commandPoolInfo, VK_NULL_HANDLE,
                                        &m_CommandPool));

        m_CommandBuffers.Resize(frameCount);
        vk::CommandBufferAllocateInfo bufferAlloc{};
        bufferAlloc.sType       = vk::StructureType::eCommandBufferAllocateInfo;
        bufferAlloc.commandPool = m_CommandPool;
        bufferAlloc.level       = vk::CommandBufferLevel::eSecondary;
        bufferAlloc.commandBufferCount = frameCount;

        VkCall(device.allocateCommandBuffers(&bufferAlloc,
                                             m_CommandBuffers.Raw()));
    }
    void VulkanImGuiLayer::OnShutdownRendererInterface()
    {
        vk::Device device = VulkanRenderer::Device();
        device.waitIdle();

        device.destroyCommandPool(m_CommandPool);

        ImGui_ImplVulkan_Shutdown();
        vkDestroyDescriptorPool(device, m_DescriptorPool, VK_NULL_HANDLE);
    }

    void VulkanImGuiLayer::OnNewFrame() { ImGui_ImplVulkan_NewFrame(); }
    void VulkanImGuiLayer::OnRenderDrawData()
    {
        auto&             window    = m_MainWindow;
        auto              swapChain = window.SwapChain().As<VulkanSwapChain>();
        vk::CommandBuffer commandBuffer = swapChain->CurrentCommandBuffer();

#if 0
        auto         currentImageIndex = swapChain.CurrentImageIndex();
        vk::Extent2D extent            = swapChain.Extent();

        vk::RenderingAttachmentInfoKHR colorAttachmentInfo{};
        colorAttachmentInfo.sType
            = vk::StructureType::eRenderingAttachmentInfoKHR;
        colorAttachmentInfo.imageView
            = swapChain.Frames()[currentImageIndex].ImageView;
        colorAttachmentInfo.imageLayout = vk::ImageLayout::eAttachmentOptimal;
        colorAttachmentInfo.loadOp      = vk::AttachmentLoadOp::eLoad;
        colorAttachmentInfo.storeOp     = vk::AttachmentStoreOp::eStore;

        vk::RenderingInfoKHR renderingInfo{};
        renderingInfo.sType             = vk::StructureType::eRenderingInfoKHR;
        renderingInfo.renderArea.offset = vk::Offset2D(0, 0);
        renderingInfo.renderArea.extent = extent;
        renderingInfo.layerCount        = 1;
        renderingInfo.colorAttachmentCount = 1;
        renderingInfo.pColorAttachments    = &colorAttachmentInfo;

        commandBuffer.beginRenderingKHR(&renderingInfo);
#endif

        ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), commandBuffer);

#if 0
        commandBuffer.endRenderingKHR();
#endif
    }

    i32 VulkanImGuiLayer::CreateVulkanSurface(ImGuiViewport* viewport, ImU64,
                                              const void*, ImU64* outVkSurface)
    {
        ImGuiViewportData* data = GetViewportData(viewport);
        auto          surface   = reinterpret_cast<VkSurfaceKHR*>(outVkSurface);

        VulkanSurface surf;
        surf.Create(data->Window, VulkanRenderer::PhysicalDevice());
        *surface = vk::SurfaceKHR(surf);

        return static_cast<i32>(VK_SUCCESS);
    }
}; // namespace Vortex
