/*
 * Created by v1tr10l7 on 19.07.2024.
 * Copyright (c) 2024-2024, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#include "vtpch.hpp"

#include "Vortex/Graphics/API/Vulkan/VulkanContext.hpp"
#include "Vortex/Graphics/API/Vulkan/VulkanSwapChain.hpp"

namespace Vortex
{

    void VulkanSwapChain::CreateSurface(IWindow* windowHandle)
    {
        m_Surface.Create(windowHandle, VulkanContext::GetPhysicalDevice());
    }

    void VulkanSwapChain::Create(u32& width, u32& height, bool vsync)
    {
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

        auto indices = m_Device.GetPhysicalDevice().GetQueueFamilyIndices();
        u32  queueFamilyIndices[]
            = {indices.Graphics.value(), indices.Present.value()};
        if (indices.Graphics != indices.Present)
        {
            createInfo.imageSharingMode      = vk::SharingMode::eConcurrent;
            createInfo.queueFamilyIndexCount = 2;
            createInfo.pQueueFamilyIndices   = queueFamilyIndices;
        }

        VkCall(vk::Device(m_Device).createSwapchainKHR(&createInfo, nullptr,
                                                       &m_SwapChain));

        VkCall(vk::Device(m_Device).getSwapchainImagesKHR(
            m_SwapChain, &imageCount, nullptr));

        m_Images.resize(imageCount);
        VkCall(vk::Device(m_Device).getSwapchainImagesKHR(
            m_SwapChain, &imageCount, m_Images.data()));

        if (oldSwapChain)
        {
            for (auto imageView : m_ImageViews)
                vk::Device(m_Device).destroyImageView(imageView, nullptr);
            vk::Device(m_Device).destroySwapchainKHR(oldSwapChain, nullptr);
        }

        CreateImageViews();
    }
    void VulkanSwapChain::Destroy()
    {
        for (auto imageView : m_ImageViews)
            vk::Device(m_Device).destroyImageView(imageView, nullptr);

        vk::Device(m_Device).destroySwapchainKHR(m_SwapChain, nullptr);
    }

    void VulkanSwapChain::CreateImageViews()
    {

        m_ImageViews.resize(m_Images.size());

        for (size_t i = 0; i < m_Images.size(); i++)
        {
            vk::ImageViewCreateInfo createInfo{};
            createInfo.sType        = vk::StructureType::eImageViewCreateInfo;
            createInfo.image        = m_Images[i];
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

            VkCall(vk::Device(m_Device).createImageView(&createInfo, nullptr,
                                                        &m_ImageViews[i]));
        }
    }

    vk::Extent2D VulkanSwapChain::ChooseSwapExtent(
        const vk::SurfaceCapabilitiesKHR& capabilities)
    {
        if (capabilities.currentExtent.width
            != std::numeric_limits<uint32_t>::max())
            return capabilities.currentExtent;
        int width, height;
        glfwGetFramebufferSize(m_Surface.GetNativeWindowHandle(), &width,
                               &height);

        vk::Extent2D actualExtent
            = {static_cast<uint32_t>(width), static_cast<uint32_t>(height)};

        actualExtent.width
            = std::clamp(actualExtent.width, capabilities.minImageExtent.width,
                         capabilities.maxImageExtent.width);
        actualExtent.height = std::clamp(actualExtent.height,
                                         capabilities.minImageExtent.height,
                                         capabilities.maxImageExtent.height);

        return actualExtent;
    }

} // namespace Vortex
