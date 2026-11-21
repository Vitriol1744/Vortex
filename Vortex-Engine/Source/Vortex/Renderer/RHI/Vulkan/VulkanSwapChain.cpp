/*
 * Created by v1tr10l7 on 18.07.2024.
 * Copyright (c) 2024-2026, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#include <Vortex/Renderer/RHI/Vulkan/VulkanCommon.hpp>
#include <Vortex/Renderer/RHI/Vulkan/VulkanDevice.hpp>
#include <Vortex/Renderer/RHI/Vulkan/VulkanResources.hpp>
#include <Vortex/Renderer/RHI/Vulkan/VulkanSwapChain.hpp>

#include <Vortex/Window/Window.hpp>

#include <limits>

import Prism.String.Formatter;

namespace Vortex::Vulkan
{
    using namespace RHI;
    using Deletion = DeferredDeletion::Type;

    VulkanSwapChain::VulkanSwapChain(const SwapChainDesc& desc)
        : SwapChain(desc.Window)
        , m_VSync(desc.VSync)
        , m_PreferredFormat(desc.PreferredFormat)
    {
        if (!m_Surface.Create(desc.Window)) return;

        auto& device = VulkanDevice::Get();
        vk::SemaphoreCreateInfo semaphoreInfo{};
        m_AcquireSemaphores.Resize(device.FramesInFlight());
        for (auto& semaphore : m_AcquireSemaphores)
            VkCall(device.Handle().createSemaphore(&semaphoreInfo, nullptr,
                                                   &semaphore));

        ChooseFormat();
        m_Valid = true;
        // The images are created by the first acquire, on the render thread:
        // creation waits for the GPU, which only the thread that submits may
        // do (queue access is externally synchronized).
        m_NeedsRecreate = true;
    }

    VulkanSwapChain::~VulkanSwapChain()
    {
        if (!m_Valid) return;
        auto& device = VulkanDevice::Get();
        // Destroyed right away, swapchain before surface, while the window
        // (which the surface points into) still exists; deferring would free
        // them after the window is gone. The GPU is idle, so it's safe.
        device.WaitIdle();
        m_Images.Clear();
        vk::Device handle = device.Handle();
        for (auto semaphore : m_AcquireSemaphores) handle.destroySemaphore(semaphore);
        for (auto semaphore : m_PresentSemaphores) handle.destroySemaphore(semaphore);
        m_AcquireSemaphores.Clear();
        m_PresentSemaphores.Clear();
        if (m_SwapChain) handle.destroySwapchainKHR(m_SwapChain);
        m_SwapChain = VK_NULL_HANDLE;
        m_Surface.Destroy();
    }

    void VulkanSwapChain::DestroySemaphores(Vector<vk::Semaphore>& semaphores)
    {
        auto& device = VulkanDevice::Get();
        for (auto semaphore : semaphores)
            device.Defer(Deletion::eSemaphore, semaphore);
        semaphores.Clear();
    }

    void VulkanSwapChain::ChooseFormat()
    {
        auto formats  = m_Surface.Formats();
        auto wanted   = ToVk(m_PreferredFormat);
        m_Format      = FromVk(formats[0].format);
        m_ColorSpace  = formats[0].colorSpace;
        for (const auto& format : formats)
        {
            if (format.format == wanted
                && format.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear)
            {
                m_Format     = m_PreferredFormat;
                m_ColorSpace = format.colorSpace;
                return;
            }
        }
        VtCoreWarn("Vulkan: Preferred swapchain format unavailable, using {}",
                   vk::to_string(formats[0].format));
    }

    vk::PresentModeKHR VulkanSwapChain::ChoosePresentMode() const
    {
        // FIFO is the only mode every implementation must support.
        if (m_VSync) return vk::PresentModeKHR::eFifo;
        auto modes = m_Surface.PresentModes();
        for (auto mode : modes)
            if (mode == vk::PresentModeKHR::eMailbox) return mode;
        for (auto mode : modes)
            if (mode == vk::PresentModeKHR::eImmediate) return mode;
        return vk::PresentModeKHR::eFifo;
    }

    void VulkanSwapChain::SetVSync(bool vsync)
    {
        if (vsync == m_VSync) return;
        m_VSync         = vsync;
        m_NeedsRecreate = true;
    }

    bool VulkanSwapChain::Recreate()
    {
        auto& device       = VulkanDevice::Get();
        auto  capabilities = m_Surface.Capabilities();

        // Wayland leaves the extent to the client (currentExtent = ~0u).
        Extent2D extent{capabilities.currentExtent.width,
                        capabilities.currentExtent.height};
        if (extent.Width == std::numeric_limits<u32>::max())
        {
            auto size = m_Window->FramebufferSize();
            extent    = {static_cast<u32>(size.x > 0 ? size.x : 0),
                         static_cast<u32>(size.y > 0 ? size.y : 0)};
            auto clamp = [](u32 value, u32 low, u32 high)
            { return value < low ? low : (value > high ? high : value); };
            extent.Width  = clamp(extent.Width, capabilities.minImageExtent.width,
                                  capabilities.maxImageExtent.width);
            extent.Height = clamp(extent.Height,
                                  capabilities.minImageExtent.height,
                                  capabilities.maxImageExtent.height);
        }
        if (extent.Empty()) return false;

        // Images of the old swapchain may still be in flight.
        device.WaitIdle();

        u32 imageCount = capabilities.minImageCount + 1;
        if (capabilities.maxImageCount > 0
            && imageCount > capabilities.maxImageCount)
            imageCount = capabilities.maxImageCount;

        vk::SwapchainCreateInfoKHR info{};
        info.surface          = m_Surface.Handle();
        info.minImageCount    = imageCount;
        info.imageFormat      = ToVk(m_Format);
        info.imageColorSpace  = m_ColorSpace;
        info.imageExtent      = vk::Extent2D(extent.Width, extent.Height);
        info.imageArrayLayers = 1;
        info.imageUsage       = vk::ImageUsageFlagBits::eColorAttachment
                        | vk::ImageUsageFlagBits::eTransferDst;
        info.imageSharingMode = vk::SharingMode::eExclusive;
        info.preTransform     = capabilities.currentTransform;
        if (capabilities.supportedTransforms
            & vk::SurfaceTransformFlagBitsKHR::eIdentity)
            info.preTransform = vk::SurfaceTransformFlagBitsKHR::eIdentity;
        info.compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque;
        for (auto alpha : {vk::CompositeAlphaFlagBitsKHR::eOpaque,
                           vk::CompositeAlphaFlagBitsKHR::eInherit,
                           vk::CompositeAlphaFlagBitsKHR::ePreMultiplied,
                           vk::CompositeAlphaFlagBitsKHR::ePostMultiplied})
        {
            if (capabilities.supportedCompositeAlpha & alpha)
            {
                info.compositeAlpha = alpha;
                break;
            }
        }
        info.presentMode  = ChoosePresentMode();
        info.clipped      = VK_TRUE;
        info.oldSwapchain = m_SwapChain;

        vk::SwapchainKHR swapChain = VK_NULL_HANDLE;
        vk::Result result = device.Handle().createSwapchainKHR(&info, nullptr,
                                                               &swapChain);
        if (result != vk::Result::eSuccess)
        {
            VtCoreError("Vulkan: Failed to create the swapchain: {}",
                        VkResultToString(result));
            return false;
        }
        // The GPU is idle (above): the retired swapchain can go now, which
        // also keeps it from outliving the surface in the deletion queue.
        if (m_SwapChain) device.Handle().destroySwapchainKHR(m_SwapChain);
        m_SwapChain = swapChain;
        m_Extent    = extent;

        u32 count = 0;
        VkCall(device.Handle().getSwapchainImagesKHR(m_SwapChain, &count, nullptr));
        Vector<vk::Image> images(count);
        VkCall(device.Handle().getSwapchainImagesKHR(m_SwapChain, &count,
                                                     images.Raw()));

        m_Images.Clear();
        for (u32 i = 0; i < count; ++i)
        {
            TextureDesc textureDesc{};
            textureDesc.Format    = m_Format;
            textureDesc.Extent    = {extent.Width, extent.Height, 1};
            textureDesc.Usage     = TextureUsage::eColorAttachment
                              | TextureUsage::eTransferDst;
            textureDesc.DebugName = Prism::Format("SwapChain image {}", i);
            m_Images.PushBack(CreateRef<VulkanTexture>(textureDesc, images[i]));
        }

        DestroySemaphores(m_PresentSemaphores);
        vk::SemaphoreCreateInfo semaphoreInfo{};
        m_PresentSemaphores.Resize(count);
        for (auto& semaphore : m_PresentSemaphores)
            VkCall(device.Handle().createSemaphore(&semaphoreInfo, nullptr,
                                                   &semaphore));

        m_NeedsRecreate = false;
        VtCoreTrace("Vulkan: Swapchain {}x{}, {} images, {}", extent.Width,
                    extent.Height, count, vk::to_string(info.presentMode));
        return true;
    }

    bool VulkanSwapChain::AcquireNextImage()
    {
        auto& device = VulkanDevice::Get();
        device.CheckRenderThread();
        VtCoreAssertMsg(!m_Acquired,
                        "Vulkan: AcquireNextImage twice without presenting");
        if (!m_Valid) return false;
        if ((m_NeedsRecreate || !m_SwapChain) && !Recreate()) return false;

        m_AcquireIndex = device.FrameIndex();
        for (u32 attempt = 0; attempt < 2; ++attempt)
        {
            vk::Result result = device.Handle().acquireNextImageKHR(
                m_SwapChain, std::numeric_limits<u64>::max(),
                AcquireSemaphore(), VK_NULL_HANDLE, &m_ImageIndex);
            if (result == vk::Result::eSuccess
                || result == vk::Result::eSuboptimalKHR)
            {
                // Suboptimal still works; rebuild after presenting it.
                if (result == vk::Result::eSuboptimalKHR) m_NeedsRecreate = true;
                m_Acquired = true;
                return true;
            }
            if (result != vk::Result::eErrorOutOfDateKHR)
            {
                VtCoreError("Vulkan: Failed to acquire a swapchain image: {}",
                            VkResultToString(result));
                return false;
            }
            if (!Recreate()) return false;
        }
        return false;
    }

    ::Ref<Texture> VulkanSwapChain::CurrentImage() const
    {
        VtCoreAssertMsg(m_Acquired,
                        "Vulkan: CurrentImage() without an acquired image");
        return m_Images[m_ImageIndex];
    }

    void VulkanSwapChain::OnPresented(vk::Result result)
    {
        m_Acquired = false;
        if (result == vk::Result::eErrorOutOfDateKHR
            || result == vk::Result::eSuboptimalKHR)
            m_NeedsRecreate = true;
        else if (result != vk::Result::eSuccess)
            VtCoreError("Vulkan: Failed to present: {}", VkResultToString(result));
    }
} // namespace Vortex::Vulkan
