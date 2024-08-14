/*
 * Created by v1tr10l7 on 19.07.2024.
 * Copyright (c) 2024-2024, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#include "vtpch.hpp"

#include "Vortex/Renderer/API/Vulkan/VulkanContext.hpp"
#include "Vortex/Renderer/API/Vulkan/VulkanSwapChain.hpp"

namespace Vortex
{

    void VulkanSwapChain::CreateSurface(Window* windowHandle)
    {
        m_Surface.Create(windowHandle, VulkanContext::GetPhysicalDevice());
    }

    void VulkanSwapChain::Create(u32& width, u32& height, bool vsync)
    {
        VtCoreTrace("Vulkan: Creating swapchain...");
        // TODO(v1tr10l7): Implement VSync
        (void)vsync;
        auto                       oldSwapChain  = m_SwapChain;

        vk::SurfaceFormatKHR       surfaceFormat = m_Surface.GetFormat();
        vk::SurfaceCapabilitiesKHR capabilities  = m_Surface.GetCapabilities();
        vk::PresentModeKHR         presentMode
            = ChooseSwapPresentMode(m_Surface.GetPresentModes());
        m_ImageFormat  = surfaceFormat.format;

        m_Extent       = ChooseSwapExtent(capabilities);
        width          = m_Extent.width;
        height         = m_Extent.height;

        u32 imageCount = capabilities.minImageCount + 1;
        if (capabilities.maxImageCount > 0
            && imageCount > capabilities.maxImageCount)
            imageCount = capabilities.maxImageCount;

        vk::SwapchainCreateInfoKHR createInfo{};
        createInfo.sType           = vk::StructureType::eSwapchainCreateInfoKHR;
        createInfo.flags           = vk::SwapchainCreateFlagBitsKHR();
        createInfo.surface         = vk::SurfaceKHR(m_Surface);
        createInfo.minImageCount   = imageCount;
        createInfo.imageFormat     = surfaceFormat.format;
        createInfo.imageColorSpace = surfaceFormat.colorSpace;
        createInfo.imageExtent     = m_Extent;
        createInfo.imageArrayLayers = 1;
        createInfo.imageUsage       = vk::ImageUsageFlagBits::eColorAttachment;
        createInfo.imageSharingMode = vk::SharingMode::eExclusive;
        createInfo.queueFamilyIndexCount = 0;
        createInfo.pQueueFamilyIndices   = VK_NULL_HANDLE;
        createInfo.preTransform          = capabilities.currentTransform;
        if (capabilities.supportedTransforms
            & vk::SurfaceTransformFlagBitsKHR::eIdentity)
            createInfo.preTransform
                = vk::SurfaceTransformFlagBitsKHR::eIdentity;
        createInfo.compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque;
        createInfo.presentMode    = presentMode;
        createInfo.clipped        = VK_TRUE;
        createInfo.oldSwapchain   = oldSwapChain;

        auto indices
            = VulkanContext::GetPhysicalDevice().GetQueueFamilyIndices();
        u32 queueFamilyIndices[]
            = {indices.Graphics.value(), indices.Present.value()};
        if (indices.Graphics != indices.Present)
        {
            createInfo.imageSharingMode      = vk::SharingMode::eConcurrent;
            createInfo.queueFamilyIndexCount = 2;
            createInfo.pQueueFamilyIndices   = queueFamilyIndices;
        }

        vk::Device device = VulkanContext::GetDevice();
        VkCall(device.createSwapchainKHR(&createInfo, nullptr, &m_SwapChain));

        VkCall(device.getSwapchainImagesKHR(m_SwapChain, &imageCount, nullptr));
        m_Frames.resize(imageCount);

        std::vector<vk::Image> backbuffers(imageCount);
        VkCall(device.getSwapchainImagesKHR(m_SwapChain, &imageCount,
                                            backbuffers.data()));

        for (usize i = 0; i < imageCount; i++)
        {
            auto& frame = m_Frames[i];
            frame.Image = backbuffers[i];
        }

        if (oldSwapChain)
        {
            for (auto& frame : m_Frames)
                device.destroyImageView(frame.ImageView, nullptr);
            device.destroySwapchainKHR(oldSwapChain, nullptr);
        }

        CreateImageViews();

        CreateCommandBuffers();
        CreateSyncObjects();
        CreateRenderPass();
        CreateFramebuffers();
    }
    void VulkanSwapChain::Destroy()
    {
        VtCoreTrace("Vulkan: Destroying swapchain...");
        vk::Device device = VulkanContext::GetDevice();
        device.waitIdle();

        device.destroyRenderPass(m_RenderPass, VK_NULL_HANDLE);
        for (auto& frame : m_Frames)
        {
            VkCall(
                device.waitForFences(1, &frame.WaitFence, VK_TRUE, UINT64_MAX));
            device.destroyFence(frame.WaitFence, VK_NULL_HANDLE);
            device.destroyCommandPool(frame.CommandPool);
            device.destroySemaphore(frame.ImageAvailableSemaphore,
                                    VK_NULL_HANDLE);
            device.destroySemaphore(frame.RenderFinishedSemaphore,
                                    VK_NULL_HANDLE);
            device.destroyFramebuffer(frame.Framebuffer, VK_NULL_HANDLE);
            device.destroyImageView(frame.ImageView, nullptr);
        }

        if (m_SwapChain)
            device.destroySwapchainKHR(m_SwapChain, VK_NULL_HANDLE);
    }

    void VulkanSwapChain::BeginFrame()
    {
        m_CurrentFrameIndex = (m_CurrentFrameIndex + 1) % MAX_FRAMES_IN_FLIGHT;

        vk::Device device   = VulkanContext::GetDevice();
        VkCall(device.waitForFences(1, &GetCurrentFrame().WaitFence, VK_TRUE,
                                    UINT64_MAX));
        vk::Result result = device.acquireNextImageKHR(
            m_SwapChain, UINT64_MAX, GetCurrentFrame().ImageAvailableSemaphore,
            VK_NULL_HANDLE, &m_CurrentImageIndex);

        if (result == vk::Result::eErrorOutOfDateKHR)
        {
            OnResize(m_Extent.width, m_Extent.height);
            result = device.acquireNextImageKHR(
                m_SwapChain, UINT64_MAX,
                GetCurrentFrame().ImageAvailableSemaphore, VK_NULL_HANDLE,
                &m_CurrentImageIndex);
        }
        else if (result != vk::Result::eSuccess
                 && result != vk::Result::eSuboptimalKHR)
        {
            VkCall(result);
        }

        VkCall(device.resetFences(1, &GetCurrentFrame().WaitFence));

        GetCurrentFrame().CommandBuffer.reset(vk::CommandBufferResetFlags());

        vk::CommandBufferBeginInfo beginInfo{};
        beginInfo.sType = vk::StructureType::eCommandBufferBeginInfo;

        vk::CommandBuffer commandBuffer = GetCurrentCommandBuffer();
        VkCall(commandBuffer.begin(&beginInfo));

        vk::RenderPassBeginInfo renderPassInfo{};
        renderPassInfo.sType       = vk::StructureType::eRenderPassBeginInfo;
        renderPassInfo.renderPass  = m_RenderPass;
        renderPassInfo.framebuffer = m_Frames[m_CurrentImageIndex].Framebuffer;
        renderPassInfo.renderArea.offset.x = 0;
        renderPassInfo.renderArea.offset.y = 0;
        renderPassInfo.renderArea.extent   = m_Extent;

        vk::ClearValue clearValue;
        clearValue.setColor(vk::ClearColorValue(0.1f, 0.1f, 0.1f, 1.0f));
        renderPassInfo.clearValueCount = 1;
        renderPassInfo.pClearValues    = &clearValue;

        commandBuffer.beginRenderPass(&renderPassInfo,
                                      vk::SubpassContents::eInline);
    }
    void VulkanSwapChain::EndFrame()
    {
        vk::CommandBuffer commandBuffer = GetCurrentCommandBuffer();
        commandBuffer.endRenderPass();

        commandBuffer.end();
    }

    void VulkanSwapChain::Present()
    {
        vk::SubmitInfo submitInfo{};
        submitInfo.sType = vk::StructureType::eSubmitInfo;

        vk::Semaphore waitSemaphores[]
            = {m_Frames[m_CurrentFrameIndex].ImageAvailableSemaphore};
        vk::PipelineStageFlags waitStages[]
            = {vk::PipelineStageFlagBits::eColorAttachmentOutput};
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores    = waitSemaphores;
        submitInfo.pWaitDstStageMask  = waitStages;

        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers
            = &m_Frames[m_CurrentFrameIndex].CommandBuffer;

        vk::Semaphore signalSemaphores[]
            = {m_Frames[m_CurrentFrameIndex].RenderFinishedSemaphore};
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores    = signalSemaphores;

        vk::Queue graphicsQueue = VulkanContext::GetDevice().GetGraphicsQueue();

        VkCall(graphicsQueue.submit(1, &submitInfo,
                                    m_Frames[m_CurrentFrameIndex].WaitFence));

        vk::PresentInfoKHR presentInfo{};
        presentInfo.sType              = vk::StructureType::ePresentInfoKHR;

        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores    = signalSemaphores;

        vk::SwapchainKHR swapChains[]  = {m_SwapChain};
        presentInfo.swapchainCount     = 1;
        presentInfo.pSwapchains        = swapChains;

        presentInfo.pImageIndices      = &m_CurrentImageIndex;

        vk::Queue  presentQueue = VulkanContext::GetDevice().GetPresentQueue();
        vk::Result result       = presentQueue.presentKHR(&presentInfo);

        u32        width, height;
        width  = 0;
        height = 0;
        if (result == vk::Result::eErrorOutOfDateKHR
            || result == vk::Result::eSuboptimalKHR)
            OnResize(width, height);
        else VkCall(result);
    }
    void VulkanSwapChain::OnResize(u32 width, u32 height)
    {
        vk::Device device = VulkanContext::GetDevice();
        device.waitIdle();
        Create(width, height);

        device.waitIdle();
    }

    u32 VulkanSwapChain::AcquireNextImage()
    {
        vk::Device device   = VulkanContext::GetDevice();

        m_CurrentFrameIndex = (m_CurrentFrameIndex + 1) % MAX_FRAMES_IN_FLIGHT;
        VkCall(device.waitForFences(1, &m_Frames[m_CurrentFrameIndex].WaitFence,
                                    VK_TRUE, UINT64_MAX));

        u32        imageIndex;
        vk::Result result = device.acquireNextImageKHR(
            m_SwapChain, UINT64_MAX,
            m_Frames[m_CurrentFrameIndex].ImageAvailableSemaphore,
            VK_NULL_HANDLE, &imageIndex);

        if (result == vk::Result::eErrorOutOfDateKHR
            || result == vk::Result::eSuboptimalKHR)
        {
            OnResize(m_Extent.width, m_Extent.height);
            VkCall(device.acquireNextImageKHR(
                m_SwapChain, UINT64_MAX,
                m_Frames[m_CurrentFrameIndex].ImageAvailableSemaphore,
                VK_NULL_HANDLE, &imageIndex));
        }
        else VkCall(result);

        return imageIndex;
    }

    void VulkanSwapChain::CreateImageViews()
    {
        vk::Device device = VulkanContext::GetDevice();

        for (size_t i = 0; i < m_Frames.size(); i++)
        {
            vk::ImageViewCreateInfo createInfo{};
            createInfo.sType        = vk::StructureType::eImageViewCreateInfo;
            createInfo.image        = m_Frames[i].Image;
            createInfo.viewType     = vk::ImageViewType::e2D;
            createInfo.format       = m_Surface.GetFormat().format;
            createInfo.components.r = vk::ComponentSwizzle::eIdentity;
            createInfo.components.g = vk::ComponentSwizzle::eIdentity;
            createInfo.components.b = vk::ComponentSwizzle::eIdentity;
            createInfo.components.a = vk::ComponentSwizzle::eIdentity;
            createInfo.subresourceRange.aspectMask
                = vk::ImageAspectFlagBits::eColor;
            createInfo.subresourceRange.baseMipLevel   = 0;
            createInfo.subresourceRange.levelCount     = 1;
            createInfo.subresourceRange.baseArrayLayer = 0;
            createInfo.subresourceRange.layerCount     = 1;

            VkCall(device.createImageView(&createInfo, VK_NULL_HANDLE,
                                          &m_Frames[i].ImageView));
        }
    }

    void VulkanSwapChain::CreateCommandBuffers()
    {
        vk::Device                device = VulkanContext::GetDevice();
        vk::CommandPoolCreateInfo commandPoolInfo{};
        commandPoolInfo.sType = vk::StructureType::eCommandPoolCreateInfo;
        commandPoolInfo.pNext = VK_NULL_HANDLE;
        // TODO(v1tr10l7): should be transient:
        commandPoolInfo.flags
            = vk::CommandPoolCreateFlagBits::eResetCommandBuffer;
        commandPoolInfo.queueFamilyIndex = VulkanContext::GetPhysicalDevice()
                                               .GetQueueFamilyIndices()
                                               .Graphics.value();
        for (auto& frame : m_Frames)
        {
            if (frame.CommandPool) device.destroyCommandPool(frame.CommandPool);
            VkCall(device.createCommandPool(&commandPoolInfo, VK_NULL_HANDLE,
                                            &frame.CommandPool));
        }

        vk::CommandBufferAllocateInfo bufferInfo{};
        bufferInfo.sType = vk::StructureType::eCommandBufferAllocateInfo;
        bufferInfo.level = vk::CommandBufferLevel::ePrimary;
        bufferInfo.commandBufferCount = 1;

        for (auto& frame : m_Frames)
        {
            bufferInfo.commandPool = frame.CommandPool;
            VkCall(device.allocateCommandBuffers(&bufferInfo,
                                                 &frame.CommandBuffer));
        }
    }

    void VulkanSwapChain::CreateSyncObjects()
    {
        vk::Device              device = VulkanContext::GetDevice();

        vk::SemaphoreCreateInfo semaphoreInfo{};
        semaphoreInfo.sType = vk::StructureType::eSemaphoreCreateInfo;

        vk::FenceCreateInfo fenceInfo{};
        fenceInfo.sType = vk::StructureType::eFenceCreateInfo;
        fenceInfo.flags = vk::FenceCreateFlagBits::eSignaled;

        for (auto& frame : m_Frames)
        {
            if (frame.ImageAvailableSemaphore)
            {
                return;
                device.destroySemaphore(frame.ImageAvailableSemaphore,
                                        VK_NULL_HANDLE);
                device.destroySemaphore(frame.RenderFinishedSemaphore,
                                        VK_NULL_HANDLE);
                device.destroyFence(frame.WaitFence, VK_NULL_HANDLE);
            }

            VkCall(device.createSemaphore(&semaphoreInfo, VK_NULL_HANDLE,
                                          &frame.ImageAvailableSemaphore));
            VkCall(device.createSemaphore(&semaphoreInfo, VK_NULL_HANDLE,
                                          &frame.RenderFinishedSemaphore));

            VkCall(device.createFence(&fenceInfo, VK_NULL_HANDLE,
                                      &frame.WaitFence));
        }
    }

    void VulkanSwapChain::CreateRenderPass()
    {
        vk::Device device = VulkanContext::GetDevice();

        if (m_RenderPass)
            return; // device.destroyRenderPass(m_RenderPass, VK_NULL_HANDLE);
        vk::AttachmentDescription colorAttachmentDescription{};
        colorAttachmentDescription.format  = m_ImageFormat;
        colorAttachmentDescription.samples = vk::SampleCountFlagBits::e1;
        colorAttachmentDescription.loadOp  = vk::AttachmentLoadOp::eClear;
        colorAttachmentDescription.storeOp = vk::AttachmentStoreOp::eStore;
        colorAttachmentDescription.stencilLoadOp
            = vk::AttachmentLoadOp::eDontCare;
        colorAttachmentDescription.stencilStoreOp
            = vk::AttachmentStoreOp::eDontCare;
        colorAttachmentDescription.initialLayout = vk::ImageLayout::eUndefined;
        colorAttachmentDescription.finalLayout
            = vk::ImageLayout::ePresentSrcKHR;

        vk::AttachmentReference colorReference{};
        colorReference.attachment = 0;
        colorReference.layout     = vk::ImageLayout::eColorAttachmentOptimal;

        vk::SubpassDescription subpassDescription{};
        subpassDescription.pipelineBindPoint = vk::PipelineBindPoint::eGraphics;
        subpassDescription.colorAttachmentCount = 1;
        subpassDescription.pColorAttachments    = &colorReference;

        vk::SubpassDependency dependency{};
        dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
        dependency.dstSubpass = 0;
        dependency.srcStageMask
            = vk::PipelineStageFlagBits::eColorAttachmentOutput;
        dependency.srcAccessMask = vk::AccessFlags();
        dependency.dstStageMask
            = vk::PipelineStageFlagBits::eColorAttachmentOutput;
        dependency.dstAccessMask = vk::AccessFlagBits::eColorAttachmentWrite;

        vk::RenderPassCreateInfo renderPassInfo{};
        renderPassInfo.sType = vk::StructureType::eRenderPassCreateInfo;
        renderPassInfo.attachmentCount = 1;
        renderPassInfo.pAttachments    = &colorAttachmentDescription;
        renderPassInfo.subpassCount    = 1;
        renderPassInfo.pSubpasses      = &subpassDescription;
        renderPassInfo.dependencyCount = 1;
        renderPassInfo.pDependencies   = &dependency;

        VkCall(device.createRenderPass(&renderPassInfo, VK_NULL_HANDLE,
                                       &m_RenderPass));
    }
    void VulkanSwapChain::CreateFramebuffers()
    {
        vk::Device device = VulkanContext::GetDevice();

        for (auto& frame : m_Frames)
        {
            if (!frame.Framebuffer) break;
            device.destroyFramebuffer(frame.Framebuffer, VK_NULL_HANDLE);
            frame.Framebuffer = VK_NULL_HANDLE;
        }

        for (auto& frame : m_Frames)
        {
            vk::ImageView             attachments[] = {frame.ImageView};

            vk::FramebufferCreateInfo framebufferInfo{};
            framebufferInfo.sType = vk::StructureType::eFramebufferCreateInfo;
            framebufferInfo.renderPass      = m_RenderPass;
            framebufferInfo.attachmentCount = 1;
            framebufferInfo.pAttachments    = attachments;
            framebufferInfo.width           = m_Extent.width;
            framebufferInfo.height          = m_Extent.height;
            framebufferInfo.layers          = 1;

            VkCall(device.createFramebuffer(&framebufferInfo, VK_NULL_HANDLE,
                                            &frame.Framebuffer));
        }
    }

    vk::Extent2D VulkanSwapChain::ChooseSwapExtent(
        const vk::SurfaceCapabilitiesKHR& capabilities)
    {
        if (capabilities.currentExtent.width
            != std::numeric_limits<uint32_t>::max())
            return capabilities.currentExtent;
        auto         size = m_Surface.GetWindow()->GetFramebufferSize();

        vk::Extent2D actualExtent
            = {static_cast<uint32_t>(size.x), static_cast<uint32_t>(size.y)};

        actualExtent.width
            = std::clamp(actualExtent.width, capabilities.minImageExtent.width,
                         capabilities.maxImageExtent.width);
        actualExtent.height = std::clamp(actualExtent.height,
                                         capabilities.minImageExtent.height,
                                         capabilities.maxImageExtent.height);

        return actualExtent;
    }

} // namespace Vortex
