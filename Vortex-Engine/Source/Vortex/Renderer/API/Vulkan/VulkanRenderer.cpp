/*
 * Created by v1tr10l7 on 13.08.2024.
 * Copyright (c) 2024-2024, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#include <Vortex/Renderer/API/Vulkan/VulkanGraphicsPipeline.hpp>
#include <Vortex/Renderer/API/Vulkan/VulkanIndexBuffer.hpp>
#include <Vortex/Renderer/API/Vulkan/VulkanMaterial.hpp>
#include <Vortex/Renderer/API/Vulkan/VulkanRenderer.hpp>
#include <Vortex/Renderer/API/Vulkan/VulkanVertexBuffer.hpp>

#define VtAssertFrameStarted()                                                 \
    VtCoreAssertMsg(m_CurrentSwapChain != nullptr,                             \
                    "Did you forget to call Renderer::BeginFrame()?");

namespace Vortex
{
    static ::Ref<VulkanFramebuffer> s_CurrentFramebuffer = nullptr;

    VulkanInstance                  VulkanRenderer::s_VkInstance{};
    VulkanPhysicalDevice            VulkanRenderer::s_PhysicalDevice{};
    VulkanDevice                    VulkanRenderer::s_Device{};

    VulkanRenderer::VulkanRenderer() {}

    void VulkanRenderer::Initialize()
    {
        VtCoreAssertMsg(!s_VkInstance, "Vulkan: Renderer already initialized");
        VtCoreTrace("Vulkan: Initializing the renderer");

        s_VkInstance.Initialize();
        s_PhysicalDevice = VulkanPhysicalDevice::Pick();
        s_Device.Initialize(VulkanRenderer::PhysicalDevice());
        VulkanAllocator::Initialize();

        m_MemoryBudgetProperties.sType
            = vk::StructureType::ePhysicalDeviceMemoryBudgetPropertiesEXT;
        m_MemoryProperties.sType
            = vk::StructureType::ePhysicalDeviceMemoryProperties2;
        m_MemoryProperties.pNext = &m_MemoryBudgetProperties;
    }
    void VulkanRenderer::Shutdown()
    {
        VtCoreAssertMsg(s_VkInstance,
                        "Vulkan: Renderer is not initialized, cannot shutdown");
        VtCoreTrace("Vulkan: Shutting down the renderer");

        VulkanAllocator::Shutdown();
        s_Device.Destroy();
        s_VkInstance.Destroy();
    }

    void VulkanRenderer::BeginFrame(Window& window)
    {
        m_CurrentSwapChain = window.SwapChain().As<VulkanSwapChain>();

        m_CurrentSwapChain->BeginFrame();

        vk::CommandBufferBeginInfo beginInfo{};
        beginInfo.sType            = vk::StructureType::eCommandBufferBeginInfo;
        beginInfo.pNext            = VK_NULL_HANDLE;
        beginInfo.flags            = vk::CommandBufferUsageFlagBits();
        beginInfo.pInheritanceInfo = VK_NULL_HANDLE;

        vk::CommandBuffer commandBuffer
            = m_CurrentSwapChain->CurrentCommandBuffer();
        VkCall(commandBuffer.begin(&beginInfo));
    }
    void VulkanRenderer::EndFrame()
    {
        VtAssertFrameStarted();

        vk::CommandBuffer commandBuffer
            = m_CurrentSwapChain->CurrentCommandBuffer();
        commandBuffer.end();

        m_CurrentSwapChain = nullptr;
    }

    void VulkanRenderer::BeginRenderPass(::Ref<Framebuffer> target)
    {
        VtAssertFrameStarted();

        vk::RenderPassBeginInfo renderPassInfo{};
        renderPassInfo.sType = vk::StructureType::eRenderPassBeginInfo;
        auto         vulkanFramebuffer = target.As<VulkanFramebuffer>();
        vk::Extent2D extent            = vulkanFramebuffer->Extent();

        renderPassInfo.renderPass      = vulkanFramebuffer->RenderPass();
        s_CurrentFramebuffer           = vulkanFramebuffer;

        //TODO(v1tr10l7): Check if it is swapchain framebuffer, if not, use the framebuffer from the target

        renderPassInfo.framebuffer
            = vulkanFramebuffer->GetVulkanFramebuffer();
        renderPassInfo.renderArea.offset.x = 0;
        renderPassInfo.renderArea.offset.y = 0;
        renderPassInfo.renderArea.extent   = extent;

        Array<vk::ClearValue, 2> clearValues;
        std::array<f32, 4>       color = {0.03f, 0.03f, 0.03f, 1.0f};
        clearValues[0].setColor(color);
        clearValues[1].setDepthStencil({1.0, 0});
        renderPassInfo.clearValueCount = clearValues.Size();
        renderPassInfo.pClearValues    = clearValues.Raw();

        auto commandBuffer = m_CurrentSwapChain->CurrentCommandBuffer();
        commandBuffer.beginRenderPass(&renderPassInfo,
                                      vk::SubpassContents::eInline);

        vk::Viewport viewport{};
        viewport.x        = 0.0f;
        viewport.y        = 0.0f;
        viewport.width    = static_cast<f32>(extent.width);
        viewport.height   = static_cast<f32>(extent.height);
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        commandBuffer.setViewport(0, 1, &viewport);

        vk::Rect2D scissor{};
        scissor.offset = vk::Offset2D(0, 0);
        scissor.extent = extent;
        commandBuffer.setScissor(0, 1, &scissor);
    }
    void VulkanRenderer::EndRenderPass()
    {
        VtAssertFrameStarted();
        vk::CommandBuffer commandBuffer
            = m_CurrentSwapChain->CurrentCommandBuffer();

        commandBuffer.endRenderPass();
        s_CurrentFramebuffer->NextImage();
        s_CurrentFramebuffer = nullptr;
    }

    void VulkanRenderer::Draw(::Ref<GraphicsPipeline> pipeline,
                              ::Ref<VertexBuffer>     vertexBuffer,
                              ::Ref<IndexBuffer> indexBuffer, u32 indexCount,
                              ::Ref<Material> material)
    {
        VtAssertFrameStarted();

        vk::CommandBuffer commandBuffer
            = m_CurrentSwapChain->CurrentCommandBuffer();
        auto vkPipeline     = pipeline.As<VulkanGraphicsPipeline>();
        auto vkVertexBuffer = vertexBuffer.As<VulkanVertexBuffer>();
        auto vkIndexBuffer  = indexBuffer.As<VulkanIndexBuffer>();

        commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics,
                                   *vkPipeline);

        vk::Buffer     vertexBuffers[] = {vkVertexBuffer->Buffer()};
        vk::DeviceSize offsets[]       = {0};
        commandBuffer.bindVertexBuffers(0, 1, vertexBuffers, offsets);
        commandBuffer.bindIndexBuffer(vkIndexBuffer->Buffer(), 0,
                                      vk::IndexType::eUint32);

        auto currentFrame = m_CurrentSwapChain->CurrentFrameIndex();
        vk::DescriptorSet set
            = vkPipeline->Shader()->DescriptorSets()[0].Sets[currentFrame];

        if (material)
        {
            auto vkMaterial = material.As<VulkanMaterial>();
            set             = vkMaterial->DescriptorSets()
                      .At(0)
                      .DescriptorSets[currentFrame];
        }
        commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics,
                                         vkPipeline->Layout(), 0, 1, &set, 0,
                                         VK_NULL_HANDLE);
        commandBuffer.drawIndexed(indexCount, 1, 0, 0, 0);
    }

    usize VulkanRenderer::MemoryUsage()
    {
        usize              memoryUsage = 0;

        vk::PhysicalDevice physDevice  = VulkanRenderer::PhysicalDevice();
        physDevice.getMemoryProperties2(&m_MemoryProperties);
        for (u32 i = 0; i < m_MemoryProperties.memoryProperties.memoryHeapCount;
             i++)
        {
            if (m_MemoryProperties.memoryProperties.memoryHeaps[i].flags
                & vk::MemoryHeapFlagBits::eDeviceLocal)
                memoryUsage += m_MemoryBudgetProperties.heapUsage[i];
        }
        return memoryUsage;
    }
    usize VulkanRenderer::MemoryBudget()
    {
        usize              memoryBudget = 0;

        vk::PhysicalDevice physDevice   = VulkanRenderer::PhysicalDevice();
        physDevice.getMemoryProperties2(&m_MemoryProperties);
        for (u32 i = 0; i < m_MemoryProperties.memoryProperties.memoryHeapCount;
             i++)
        {
            if (m_MemoryProperties.memoryProperties.memoryHeaps[i].flags
                & vk::MemoryHeapFlagBits::eDeviceLocal)
                memoryBudget += m_MemoryBudgetProperties.heapBudget[i];
        }

        return memoryBudget;
    }
}; // namespace Vortex
