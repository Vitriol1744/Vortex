/*
 * Created by v1tr10l7 on 19.07.2024.
 * Copyright (c) 2024-2024, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#include "vtpch.hpp"

#include "Vortex/Renderer/API/Vulkan/VulkanRenderer.hpp"
#include "Vortex/Renderer/API/Vulkan/VulkanSwapChain.hpp"
#include "Vortex/Renderer/Renderer.hpp"

namespace Vortex
{

    void VulkanSwapChain::CreateSurface(Window* windowHandle)
    {
        m_Surface.Create(windowHandle, VulkanRenderer::GetPhysicalDevice());
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

        std::array<vk::CompositeAlphaFlagBitsKHR, 4> compositeAlphaFlags = {
            vk::CompositeAlphaFlagBitsKHR::eOpaque,
            vk::CompositeAlphaFlagBitsKHR::ePreMultiplied,
            vk::CompositeAlphaFlagBitsKHR::ePostMultiplied,
            vk::CompositeAlphaFlagBitsKHR::eInherit,
        };

        // Choose composite alpha
        for (auto compositeAlpha : compositeAlphaFlags)
        {
            if (capabilities.supportedCompositeAlpha & compositeAlpha)
            {
                swapChainInfo.compositeAlpha = compositeAlpha;
                break;
            }
        }
        swapChainInfo.presentMode  = presentMode;
        swapChainInfo.clipped      = VK_TRUE;
        swapChainInfo.oldSwapchain = oldSwapChain;

        auto indices
            = VulkanRenderer::GetPhysicalDevice().GetQueueFamilyIndices();
        u32 queueFamilyIndices[]
            = {indices.Graphics.value(), indices.Present.value()};
        if (indices.Graphics != indices.Present)
        {
            swapChainInfo.imageSharingMode      = vk::SharingMode::eConcurrent;
            swapChainInfo.queueFamilyIndexCount = 2;
            swapChainInfo.pQueueFamilyIndices   = queueFamilyIndices;
        }

        vk::Device device = VulkanRenderer::GetDevice();
        VkCall(device.createSwapchainKHR(&swapChainInfo, VK_NULL_HANDLE,
                                         &m_SwapChain));

        VkCall(device.getSwapchainImagesKHR(m_SwapChain, &imageCount,
                                            VK_NULL_HANDLE));
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
                device.destroyImageView(frame.ImageView, VK_NULL_HANDLE);
            device.destroySwapchainKHR(oldSwapChain, VK_NULL_HANDLE);
        }

        CreateImageViews();
        CreateDepthBuffer();

        CreateCommandBuffers();
        CreateSyncObjects();

        if (!m_RenderPass) CreateRenderPass();
        CreateFramebuffers();
    }
    void VulkanSwapChain::Destroy()
    {
        vk::Device device = VulkanRenderer::GetDevice();
        device.waitIdle();

        device.destroyRenderPass(m_RenderPass, VK_NULL_HANDLE);
        m_RenderPass = VK_NULL_HANDLE;

        VtCoreTrace("Vulkan: Destroying swapChain frames...");
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
            device.destroyImageView(frame.ImageView, VK_NULL_HANDLE);

            std::memset(&frame, 0, sizeof(Frame));
        }

        VtCoreTrace("Vulkan: Destroying the depth buffer...");
        device.destroyImageView(m_DepthImageView);
        m_DepthImage.Destroy();

        VtCoreTrace("Vulkan: Destroying swapChain...");
        if (m_SwapChain)
            device.destroySwapchainKHR(m_SwapChain, VK_NULL_HANDLE);
        m_SwapChain = VK_NULL_HANDLE;
    }

    void VulkanSwapChain::BeginFrame()
    {

        vk::Device device   = VulkanRenderer::GetDevice();
        m_CurrentImageIndex = AcquireNextImage();

        device.resetCommandPool(GetCurrentFrame().CommandPool,
                                vk::CommandPoolResetFlags());
    }

    void VulkanSwapChain::Present()
    {
        vk::SubmitInfo         submitInfo{};
        vk::PipelineStageFlags waitStage
            = vk::PipelineStageFlagBits::eColorAttachmentOutput;

        submitInfo.sType              = vk::StructureType::eSubmitInfo;
        submitInfo.pNext              = VK_NULL_HANDLE;
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = &GetCurrentFrame().ImageAvailableSemaphore;
        submitInfo.pWaitDstStageMask    = &waitStage;
        submitInfo.commandBufferCount   = 1;
        submitInfo.pCommandBuffers      = &GetCurrentFrame().CommandBuffer;
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores
            = &GetCurrentFrame().RenderFinishedSemaphore;

        vk::Device device = VulkanRenderer::GetDevice();
        VkCall(device.resetFences(1, &GetCurrentFrame().WaitFence));

        vk::Queue graphicsQueue = VulkanRenderer::GetDevice().GetGraphicsQueue();
        VkCall(graphicsQueue.submit(1, &submitInfo,
                                    m_Frames[m_CurrentFrameIndex].WaitFence));

        vk::PresentInfoKHR presentInfo{};
        presentInfo.sType              = vk::StructureType::ePresentInfoKHR;
        presentInfo.pNext              = VK_NULL_HANDLE;
        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores
            = &GetCurrentFrame().RenderFinishedSemaphore;
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains    = &m_SwapChain;
        presentInfo.pImageIndices  = &m_CurrentImageIndex;
        presentInfo.pResults       = VK_NULL_HANDLE;

        vk::Queue  presentQueue = VulkanRenderer::GetDevice().GetPresentQueue();
        vk::Result result       = presentQueue.presentKHR(&presentInfo);

        if (result == vk::Result::eErrorOutOfDateKHR
            || result == vk::Result::eSuboptimalKHR)
            OnResize();
        else if (result != vk::Result::eSuccess)
        {
            VtCoreError("Vulkan: Failed to present swapChain image");
            VkCall(result);
        }
    }
    void VulkanSwapChain::OnResize()
    {
        vk::Device device = VulkanRenderer::GetDevice();
        device.waitIdle();
        Create(m_VSync);

        device.waitIdle();
    }

    u32 VulkanSwapChain::AcquireNextImage()
    {
        vk::Device device   = VulkanRenderer::GetDevice();

        u32 framesInFlight  = Renderer::GetConfiguration().MaxFramesInFlight;
        m_CurrentFrameIndex = (m_CurrentFrameIndex + 1) % framesInFlight;
        VkCall(device.waitForFences(1, &GetCurrentFrame().WaitFence, VK_TRUE,
                                    UINT64_MAX));

        u32        imageIndex;
        vk::Result result = device.acquireNextImageKHR(
            m_SwapChain, UINT64_MAX,
            m_Frames[m_CurrentFrameIndex].ImageAvailableSemaphore,
            VK_NULL_HANDLE, &imageIndex);

        if (result == vk::Result::eErrorOutOfDateKHR
            || result == vk::Result::eSuboptimalKHR)
        {
            OnResize();
            VkCall(device.acquireNextImageKHR(
                m_SwapChain, UINT64_MAX,
                m_Frames[m_CurrentFrameIndex].ImageAvailableSemaphore,
                VK_NULL_HANDLE, &imageIndex));
        }
        else if (result != vk::Result::eSuccess)
        {
            VtCoreError("Vulkan: Failed to acquire next swapChain image");
            VkCall(result);
        }

        return imageIndex;
    }

    void VulkanSwapChain::CreateImageViews()
    {
        vk::Device device = VulkanRenderer::GetDevice();

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
        vk::Device                device = VulkanRenderer::GetDevice();
        vk::CommandPoolCreateInfo commandPoolInfo{};
        commandPoolInfo.sType = vk::StructureType::eCommandPoolCreateInfo;
        commandPoolInfo.pNext = VK_NULL_HANDLE;
        commandPoolInfo.flags = vk::CommandPoolCreateFlagBits::eTransient;
        commandPoolInfo.queueFamilyIndex = VulkanRenderer::GetPhysicalDevice()
                                               .GetQueueFamilyIndices()
                                               .Graphics.value();

        vk::CommandBufferAllocateInfo commandBufferInfo{};
        commandBufferInfo.sType = vk::StructureType::eCommandBufferAllocateInfo;
        commandBufferInfo.pNext = VK_NULL_HANDLE;
        commandBufferInfo.commandPool        = VK_NULL_HANDLE;
        commandBufferInfo.level              = vk::CommandBufferLevel::ePrimary;
        commandBufferInfo.commandBufferCount = 1;

        for (auto& frame : m_Frames)
        {
            if (frame.CommandPool) device.destroyCommandPool(frame.CommandPool);
            frame.CommandPool = VK_NULL_HANDLE;
            VkCall(device.createCommandPool(&commandPoolInfo, VK_NULL_HANDLE,
                                            &frame.CommandPool));

            commandBufferInfo.commandPool = frame.CommandPool;
            VkCall(device.allocateCommandBuffers(&commandBufferInfo,
                                                 &frame.CommandBuffer));
        }
    }

    void VulkanSwapChain::CreateSyncObjects()
    {
        vk::Device              device = VulkanRenderer::GetDevice();

        vk::SemaphoreCreateInfo semaphoreInfo{};
        semaphoreInfo.sType = vk::StructureType::eSemaphoreCreateInfo;
        semaphoreInfo.pNext = VK_NULL_HANDLE;
        semaphoreInfo.flags = vk::SemaphoreCreateFlags();

        vk::FenceCreateInfo fenceInfo{};
        fenceInfo.sType = vk::StructureType::eFenceCreateInfo;
        fenceInfo.pNext = VK_NULL_HANDLE;
        fenceInfo.flags = vk::FenceCreateFlagBits::eSignaled;

        for (auto& frame : m_Frames)
        {
            if (!frame.ImageAvailableSemaphore)
                VkCall(device.createSemaphore(&semaphoreInfo, VK_NULL_HANDLE,
                                              &frame.ImageAvailableSemaphore));
            if (!frame.RenderFinishedSemaphore)
                VkCall(device.createSemaphore(&semaphoreInfo, VK_NULL_HANDLE,
                                              &frame.RenderFinishedSemaphore));
            if (!frame.WaitFence)
                VkCall(device.createFence(&fenceInfo, VK_NULL_HANDLE,
                                          &frame.WaitFence));
        }
    }

    void VulkanSwapChain::CreateRenderPass()
    {
        vk::Device                device = VulkanRenderer::GetDevice();

        vk::AttachmentDescription colorAttachment{};
        colorAttachment.flags          = vk::AttachmentDescriptionFlags();
        colorAttachment.format         = m_ImageFormat;
        colorAttachment.samples        = vk::SampleCountFlagBits::e1;
        colorAttachment.loadOp         = vk::AttachmentLoadOp::eClear;
        colorAttachment.storeOp        = vk::AttachmentStoreOp::eStore;
        colorAttachment.stencilLoadOp  = vk::AttachmentLoadOp::eDontCare;
        colorAttachment.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
        colorAttachment.initialLayout  = vk::ImageLayout::eUndefined;
        colorAttachment.finalLayout    = vk::ImageLayout::ePresentSrcKHR;

        vk::Format depthFormat
            = VulkanRenderer::GetPhysicalDevice().FindDepthFormat();
        vk::AttachmentDescription depthAttachment{};
        depthAttachment.flags          = vk::AttachmentDescriptionFlags();
        depthAttachment.format         = depthFormat;
        depthAttachment.samples        = vk::SampleCountFlagBits::e1;
        depthAttachment.loadOp         = vk::AttachmentLoadOp::eClear;
        depthAttachment.storeOp        = vk::AttachmentStoreOp::eDontCare;
        depthAttachment.stencilLoadOp  = vk::AttachmentLoadOp::eDontCare;
        depthAttachment.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
        depthAttachment.initialLayout  = vk::ImageLayout::eUndefined;
        depthAttachment.finalLayout
            = vk::ImageLayout::eDepthStencilAttachmentOptimal;

        vk::AttachmentReference colorReference{};
        colorReference.attachment = 0;
        colorReference.layout     = vk::ImageLayout::eColorAttachmentOptimal;

        vk::AttachmentReference depthReference{};
        depthReference.attachment = 1;
        depthReference.layout = vk::ImageLayout::eDepthStencilAttachmentOptimal;

        vk::SubpassDescription subpassDescription{};
        subpassDescription.flags             = vk::SubpassDescriptionFlags();
        subpassDescription.pipelineBindPoint = vk::PipelineBindPoint::eGraphics;
        subpassDescription.inputAttachmentCount    = 0;
        subpassDescription.pInputAttachments       = VK_NULL_HANDLE;
        subpassDescription.colorAttachmentCount    = 1;
        subpassDescription.pColorAttachments       = &colorReference;
        subpassDescription.pResolveAttachments     = VK_NULL_HANDLE;
        subpassDescription.pDepthStencilAttachment = &depthReference;
        subpassDescription.preserveAttachmentCount = 0;
        subpassDescription.pPreserveAttachments    = VK_NULL_HANDLE;

        vk::SubpassDependency dependency{};
        dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
        dependency.dstSubpass = 0;
        dependency.srcStageMask
            = vk::PipelineStageFlagBits::eColorAttachmentOutput
            | vk::PipelineStageFlagBits::eEarlyFragmentTests;
        dependency.dstStageMask
            = vk::PipelineStageFlagBits::eColorAttachmentOutput
            | vk::PipelineStageFlagBits::eEarlyFragmentTests;
        dependency.srcAccessMask = vk::AccessFlags();
        dependency.dstAccessMask
            = vk::AccessFlagBits::eColorAttachmentWrite
            | vk::AccessFlagBits::eDepthStencilAttachmentWrite;
        dependency.dependencyFlags = vk::DependencyFlags();

        std::array<vk::AttachmentDescription, 2> attachments
            = {colorAttachment, depthAttachment};

        vk::RenderPassCreateInfo renderPassInfo{};
        renderPassInfo.sType = vk::StructureType::eRenderPassCreateInfo;
        renderPassInfo.pNext = VK_NULL_HANDLE;
        renderPassInfo.flags = vk::RenderPassCreateFlags();
        renderPassInfo.attachmentCount = attachments.size();
        renderPassInfo.pAttachments    = attachments.data();
        renderPassInfo.subpassCount    = 1;
        renderPassInfo.pSubpasses      = &subpassDescription;
        renderPassInfo.dependencyCount = 1;
        renderPassInfo.pDependencies   = &dependency;

        VkCall(device.createRenderPass(&renderPassInfo, VK_NULL_HANDLE,
                                       &m_RenderPass));
    }
    void VulkanSwapChain::CreateFramebuffers()
    {
        vk::Device device = VulkanRenderer::GetDevice();

        for (auto& frame : m_Frames)
        {
            if (!frame.Framebuffer) break;
            device.destroyFramebuffer(frame.Framebuffer, VK_NULL_HANDLE);
            frame.Framebuffer = VK_NULL_HANDLE;
        }

        for (auto& frame : m_Frames)
        {
            std::array<vk::ImageView, 2> attachments
                = {frame.ImageView, m_DepthImageView};

            vk::FramebufferCreateInfo framebufferInfo{};
            framebufferInfo.sType = vk::StructureType::eFramebufferCreateInfo;
            framebufferInfo.pNext = VK_NULL_HANDLE;
            framebufferInfo.flags = vk::FramebufferCreateFlags();
            framebufferInfo.renderPass      = m_RenderPass;
            framebufferInfo.attachmentCount = attachments.size();
            framebufferInfo.pAttachments    = attachments.data();
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
            = VulkanRenderer::GetPhysicalDevice().FindDepthFormat();

        vk::Device device = VulkanRenderer::GetDevice();
        if (m_DepthImage)
        {
            device.destroyImageView(m_DepthImageView);
            m_DepthImage.Destroy();
        }
        m_DepthImage.Create(m_Extent.width, m_Extent.height, depthFormat,
                            vk::ImageTiling::eOptimal,
                            vk::ImageUsageFlagBits::eDepthStencilAttachment);
        m_DepthImage.TransitionLayout(
            vk::ImageLayout::eUndefined,
            vk::ImageLayout::eDepthStencilAttachmentOptimal);

        vk::ImageViewCreateInfo viewInfo{};
        viewInfo.sType        = vk::StructureType::eImageViewCreateInfo;
        viewInfo.pNext        = VK_NULL_HANDLE;
        viewInfo.flags        = vk::ImageViewCreateFlags();
        viewInfo.image        = m_DepthImage;
        viewInfo.viewType     = vk::ImageViewType::e2D;
        viewInfo.format       = depthFormat;
        viewInfo.components.r = vk::ComponentSwizzle::eIdentity;
        viewInfo.components.g = vk::ComponentSwizzle::eIdentity;
        viewInfo.components.b = vk::ComponentSwizzle::eIdentity;
        viewInfo.components.a = vk::ComponentSwizzle::eIdentity;
        viewInfo.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eDepth;
        viewInfo.subresourceRange.baseMipLevel   = 0;
        viewInfo.subresourceRange.levelCount     = 1;
        viewInfo.subresourceRange.baseArrayLayer = 0;
        viewInfo.subresourceRange.layerCount     = 1;

        VkCall(device.createImageView(&viewInfo, VK_NULL_HANDLE,
                                      &m_DepthImageView));
    }

    vk::Extent2D VulkanSwapChain::ChooseSwapExtent(
        const vk::SurfaceCapabilitiesKHR& capabilities)
    {
        if (capabilities.currentExtent.width != std::numeric_limits<u32>::max())
            return capabilities.currentExtent;
        auto         size = m_Surface.GetWindow()->GetFramebufferSize();

        vk::Extent2D actualExtent
            = {static_cast<u32>(size.x), static_cast<u32>(size.y)};

        actualExtent.width
            = std::clamp(actualExtent.width, capabilities.minImageExtent.width,
                         capabilities.maxImageExtent.width);
        actualExtent.height = std::clamp(actualExtent.height,
                                         capabilities.minImageExtent.height,
                                         capabilities.maxImageExtent.height);

        return actualExtent;
    }

} // namespace Vortex
