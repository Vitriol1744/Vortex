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

    void VulkanSwapChain::Create(bool vsync)
    {
        VtCoreTrace("Vulkan: Creating swapchain...");
        auto                       oldSwapChain  = m_SwapChain;

        vk::SurfaceFormatKHR       surfaceFormat = m_Surface.GetFormat();
        vk::SurfaceCapabilitiesKHR capabilities  = m_Surface.GetCapabilities();
        vk::PresentModeKHR         presentMode
            = ChooseSwapPresentMode(m_Surface.GetPresentModes());
        if (vsync) presentMode = vk::PresentModeKHR::eFifo;

        m_ImageFormat  = surfaceFormat.format;
        m_Extent       = ChooseSwapExtent(capabilities);

        u32 imageCount = capabilities.minImageCount + 1;
        if (capabilities.maxImageCount > 0
            && imageCount > capabilities.maxImageCount)
            imageCount = capabilities.maxImageCount;

        vk::SwapchainCreateInfoKHR swapChainInfo{};
        swapChainInfo.sType   = vk::StructureType::eSwapchainCreateInfoKHR;
        swapChainInfo.pNext   = VK_NULL_HANDLE;
        swapChainInfo.flags   = vk::SwapchainCreateFlagBitsKHR();
        swapChainInfo.surface = vk::SurfaceKHR(m_Surface);
        swapChainInfo.minImageCount    = imageCount;
        swapChainInfo.imageFormat      = surfaceFormat.format;
        swapChainInfo.imageColorSpace  = surfaceFormat.colorSpace;
        swapChainInfo.imageExtent      = m_Extent;
        swapChainInfo.imageArrayLayers = 1;
        swapChainInfo.imageUsage = vk::ImageUsageFlagBits::eColorAttachment;
        swapChainInfo.imageSharingMode      = vk::SharingMode::eExclusive;
        swapChainInfo.queueFamilyIndexCount = 0;
        swapChainInfo.pQueueFamilyIndices   = VK_NULL_HANDLE;
        swapChainInfo.preTransform          = capabilities.currentTransform;
        if (capabilities.supportedTransforms
            & vk::SurfaceTransformFlagBitsKHR::eIdentity)
            swapChainInfo.preTransform
                = vk::SurfaceTransformFlagBitsKHR::eIdentity;
        swapChainInfo.compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque;
        swapChainInfo.presentMode    = presentMode;
        swapChainInfo.clipped        = VK_TRUE;
        swapChainInfo.oldSwapchain   = oldSwapChain;

        auto indices
            = VulkanContext::GetPhysicalDevice().GetQueueFamilyIndices();
        u32 queueFamilyIndices[]
            = {indices.Graphics.value(), indices.Present.value()};
        if (indices.Graphics != indices.Present)
        {
            swapChainInfo.imageSharingMode      = vk::SharingMode::eConcurrent;
            swapChainInfo.queueFamilyIndexCount = 2;
            swapChainInfo.pQueueFamilyIndices   = queueFamilyIndices;
        }

        vk::Device device = VulkanContext::GetDevice();
        VkCall(
            device.createSwapchainKHR(&swapChainInfo, nullptr, &m_SwapChain));

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
            OnResize();
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

        vk::ClearValue     clearValue;
        std::array<f32, 4> color = {0.1f, 0.1f, 0.1f, 1.0f};
        clearValue.setColor(color);
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
        presentInfo.pNext              = VK_NULL_HANDLE;
        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores    = signalSemaphores;
        presentInfo.swapchainCount     = 1;
        presentInfo.pSwapchains        = &m_SwapChain;
        presentInfo.pImageIndices      = &m_CurrentImageIndex;
        presentInfo.pResults           = VK_NULL_HANDLE;

        vk::Queue  presentQueue = VulkanContext::GetDevice().GetPresentQueue();
        vk::Result result       = presentQueue.presentKHR(&presentInfo);

        if (result == vk::Result::eErrorOutOfDateKHR
            || result == vk::Result::eSuboptimalKHR)
            OnResize();
        else VkCall(result);
    }
    void VulkanSwapChain::OnResize()
    {
        vk::Device device = VulkanContext::GetDevice();
        device.waitIdle();
        Create(false);

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

        if (result == vk::Result::eErrorOutOfDateKHR)
        {
            OnResize();
            VkCall(device.acquireNextImageKHR(
                m_SwapChain, UINT64_MAX,
                m_Frames[m_CurrentFrameIndex].ImageAvailableSemaphore,
                VK_NULL_HANDLE, &imageIndex));
        }
        else if (result != vk::Result::eSuboptimalKHR) VkCall(result);

        return imageIndex;
    }

    void VulkanSwapChain::CreateImageViews()
    {
        vk::Device device = VulkanContext::GetDevice();

        for (size_t i = 0; i < m_Frames.size(); i++)
        {
            vk::ImageViewCreateInfo viewInfo{};
            viewInfo.sType        = vk::StructureType::eImageViewCreateInfo;
            viewInfo.pNext        = VK_NULL_HANDLE;
            viewInfo.flags        = vk::ImageViewCreateFlags();
            viewInfo.image        = m_Frames[i].Image;
            viewInfo.viewType     = vk::ImageViewType::e2D;
            viewInfo.format       = m_Surface.GetFormat().format;
            viewInfo.components.r = vk::ComponentSwizzle::eIdentity;
            viewInfo.components.g = vk::ComponentSwizzle::eIdentity;
            viewInfo.components.b = vk::ComponentSwizzle::eIdentity;
            viewInfo.components.a = vk::ComponentSwizzle::eIdentity;
            viewInfo.subresourceRange.aspectMask
                = vk::ImageAspectFlagBits::eColor;
            viewInfo.subresourceRange.baseMipLevel   = 0;
            viewInfo.subresourceRange.levelCount     = 1;
            viewInfo.subresourceRange.baseArrayLayer = 0;
            viewInfo.subresourceRange.layerCount     = 1;

            VkCall(device.createImageView(&viewInfo, VK_NULL_HANDLE,
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

    void VulkanSwapChain::CreateDepthBuffer()
    {
        vk::Format depthFormat
            = VulkanContext::GetPhysicalDevice().FindDepthFormat();

        vk::ImageCreateInfo imageInfo{};
        imageInfo.sType         = vk::StructureType::eImageCreateInfo;
        imageInfo.imageType     = vk::ImageType::e2D;
        imageInfo.extent.width  = m_Extent.width;
        imageInfo.extent.height = m_Extent.height;
        imageInfo.extent.depth  = 1;
        imageInfo.mipLevels     = 1;
        imageInfo.arrayLayers   = 1;
        imageInfo.format        = depthFormat;
        imageInfo.tiling        = vk::ImageTiling::eOptimal;
        imageInfo.initialLayout = vk::ImageLayout::eUndefined;
        imageInfo.usage       = vk::ImageUsageFlagBits::eDepthStencilAttachment;
        imageInfo.samples     = vk::SampleCountFlagBits::e1;
        imageInfo.sharingMode = vk::SharingMode::eExclusive;

        vk::DeviceSize size;
        m_DepthAllocation = VulkanAllocator::AllocateImage(
            imageInfo, VMA_MEMORY_USAGE_GPU_ONLY, m_DepthImage, size);

        vk::ImageViewCreateInfo viewInfo{};
        viewInfo.sType    = vk::StructureType::eImageViewCreateInfo;
        viewInfo.image    = m_DepthImage;
        viewInfo.viewType = vk::ImageViewType::e2D;
        viewInfo.format   = depthFormat;
        viewInfo.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eDepth;
        viewInfo.subresourceRange.baseMipLevel   = 0;
        viewInfo.subresourceRange.levelCount     = 1;
        viewInfo.subresourceRange.baseArrayLayer = 0;
        viewInfo.subresourceRange.layerCount     = 1;

        vk::Device device                        = VulkanContext::GetDevice();
        VkCall(device.createImageView(&viewInfo, VK_NULL_HANDLE,
                                      &m_DepthImageView));
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
