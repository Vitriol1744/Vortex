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
        (void)width;
        (void)height;
        (void)vsync;
        /*vk::SurfaceFormatKHR       surfaceFormat = m_Surface->GetFormat();
        vk::SurfaceCapabilitiesKHR capabilities  = m_Surface->GetCapabilities();
        vk::PresentModeKHR         presentMode
            = ChooseSwapPresentMode(m_Surface->GetPresentModes());
        m_Extent       = chooseSwapExtent(capabilities);

        u32 imageCount = capabilities.minImageCount + 1;
        if (capabilities.maxImageCount > 0
            && imageCount > capabilities.maxImageCount)
            imageCount = capabilities.maxImageCount;

        vk::SwapchainCreateInfoKHR createInfo{};
        createInfo.sType           = vk::StructureType::eSwapchainCreateInfoKHR;
        createInfo.surface         = vk::SurfaceKHR(*m_Surface);
        createInfo.minImageCount   = imageCount;
        createInfo.imageFormat     = surfaceFormat.format;
        createInfo.imageColorSpace = surfaceFormat.colorSpace;
        createInfo.imageExtent     = m_Extent;
        createInfo.imageArrayLayers = 1;
        createInfo.imageUsage       = vk::ImageUsageFlagBits::eColorAttachment;
        createInfo.imageSharingMode = vk::SharingMode::eExclusive;
        createInfo.preTransform     = capabilities.currentTransform;
        createInfo.compositeAlpha   = vk::CompositeAlphaFlagBitsKHR::eOpaque;
        createInfo.presentMode      = presentMode;
        createInfo.clipped          = VK_TRUE;

        VkCall(vk::Device(m_Device).createSwapchainKHR(&createInfo, nullptr,
                                                       &m_SwapChain));

        VkCall(vk::Device(m_Device).getSwapchainImagesKHR(
            m_SwapChain, &imageCount, nullptr));

        m_Images.resize(imageCount);
        VkCall(vk::Device(m_Device).getSwapchainImagesKHR(
            m_SwapChain, &imageCount, m_Images.data()));
*/
    }

} // namespace Vortex
