/*
 * Created by v1tr10l7 on 19.07.2024.
 * Copyright (c) 2024-2024, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include "Vortex/Graphics/API/Vulkan/VulkanDevice.hpp"
#include "Vortex/Graphics/API/Vulkan/VulkanSurface.hpp"

namespace Vortex
{
    class VulkanSwapChain
    {
      public:
        VulkanSwapChain() = default;

        void        CreateSurface(IWindow* windowHandle);
        void        Create(u32& width, u32& height, bool vsync = false);

        inline void DestroySurface() { m_Surface.Destroy(); }

        inline const VulkanSurface& GetSurface() const { return m_Surface; }

      private:
        VulkanSurface              m_Surface;
        VulkanDevice               m_Device;

        vk::SwapchainKHR           m_SwapChain = VK_NULL_HANDLE;
        vk::PresentModeKHR         m_PresentMode;
        vk::Extent2D               m_Extent;
        vk::Format                 m_ImageFormat;

        std::vector<vk::Image>     m_Images;
        std::vector<vk::ImageView> m_ImageViews;

        vk::PresentModeKHR         ChooseSwapPresentMode(
                    const std::vector<vk::PresentModeKHR>& availablePresentModes)
        {
            for (const auto& availablePresentMode : availablePresentModes)
            {
                if (availablePresentMode == vk::PresentModeKHR::eMailbox)
                    return availablePresentMode;
            }

            return vk::PresentModeKHR::eFifo;
        }

        vk::Extent2D
        chooseSwapExtent(const vk::SurfaceCapabilitiesKHR& capabilities)
        {
            if (capabilities.currentExtent.width
                != std::numeric_limits<uint32_t>::max())
                return capabilities.currentExtent;
            int width, height;
            glfwGetFramebufferSize(m_Surface.GetNativeWindowHandle(), &width,
                                   &height);

            vk::Extent2D actualExtent
                = {static_cast<uint32_t>(width), static_cast<uint32_t>(height)};

            actualExtent.width  = std::clamp(actualExtent.width,
                                             capabilities.minImageExtent.width,
                                             capabilities.maxImageExtent.width);
            actualExtent.height = std::clamp(
                actualExtent.height, capabilities.minImageExtent.height,
                capabilities.maxImageExtent.height);

            return actualExtent;
        }
    };
}; // namespace Vortex
