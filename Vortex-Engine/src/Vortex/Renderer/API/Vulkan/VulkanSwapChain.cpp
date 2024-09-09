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

        if (oldSwapChain)
            device.destroySwapchainKHR(oldSwapChain, VK_NULL_HANDLE);

        m_Framebuffer = CreateRef<VulkanFramebuffer>(
            m_Extent, backbuffers, m_Surface.GetFormat().format);

        CreateCommandBuffers();
        CreateSyncObjects();
    }
    void VulkanSwapChain::Destroy()
    {
        vk::Device device = VulkanRenderer::GetDevice();
        device.waitIdle();

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

            std::memset(&frame, 0, sizeof(PerFrameData));
        }

        m_Framebuffer.reset();
        VtCoreTrace("Vulkan: Destroying swapChain...");
        if (m_SwapChain)
            device.destroySwapchainKHR(m_SwapChain, VK_NULL_HANDLE);
        m_SwapChain = VK_NULL_HANDLE;
    }

    void VulkanSwapChain::BeginFrame()
    {

        vk::Device device   = VulkanRenderer::GetDevice();
        m_CurrentImageIndex = AcquireNextImage();

        device.resetCommandPool(GetCurrentFrameData().CommandPool,
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
        submitInfo.pWaitSemaphores
            = &GetCurrentFrameData().ImageAvailableSemaphore;
        submitInfo.pWaitDstStageMask    = &waitStage;
        submitInfo.commandBufferCount   = 1;
        submitInfo.pCommandBuffers      = &GetCurrentFrameData().CommandBuffer;
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores
            = &GetCurrentFrameData().RenderFinishedSemaphore;

        vk::Device device = VulkanRenderer::GetDevice();
        VkCall(device.resetFences(1, &GetCurrentFrameData().WaitFence));

        vk::Queue graphicsQueue
            = VulkanRenderer::GetDevice().GetGraphicsQueue();
        VkCall(graphicsQueue.submit(1, &submitInfo,
                                    m_Frames[m_CurrentFrameIndex].WaitFence));

        vk::PresentInfoKHR presentInfo{};
        presentInfo.sType              = vk::StructureType::ePresentInfoKHR;
        presentInfo.pNext              = VK_NULL_HANDLE;
        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores
            = &GetCurrentFrameData().RenderFinishedSemaphore;
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
        VkCall(device.waitForFences(1, &GetCurrentFrameData().WaitFence,
                                    VK_TRUE, UINT64_MAX));

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
