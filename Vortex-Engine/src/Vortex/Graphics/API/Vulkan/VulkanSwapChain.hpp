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
        struct Frame
        {
            vk::Image         Image                   = VK_NULL_HANDLE;
            vk::ImageView     ImageView               = VK_NULL_HANDLE;
            vk::CommandPool   CommandPool             = VK_NULL_HANDLE;
            vk::CommandBuffer CommandBuffer           = VK_NULL_HANDLE;
            vk::Semaphore     ImageAvailableSemaphore = VK_NULL_HANDLE;
            vk::Semaphore     RenderFinishedSemaphore = VK_NULL_HANDLE;
            vk::Fence         WaitFence               = VK_NULL_HANDLE;
        };

        VulkanSwapChain() = default;

        void Initialize(const VulkanDevice& device) { m_Device = device; }
        void CreateSurface(IWindow* windowHandle);
        void Create(u32& width, u32& height, bool vsync = false);

        void Destroy();
        inline void                 DestroySurface() { m_Surface.Destroy(); }

        void                        OnResize(u32 width, u32 height);

        inline const VulkanSurface& GetSurface() const { return m_Surface; }
        inline const std::vector<Frame>& GetFrames() const { return m_Frames; }

        inline operator vk::SwapchainKHR() const { return m_SwapChain; }
        inline vk::Extent2D GetExtent() const { return m_Extent; }
        inline vk::Format   GetImageFormat() const { return m_ImageFormat; }

        inline const std::vector<vk::Semaphore>&
        GetImageAvailableSemaphores() const
        {
            return m_ImageAvailableSemaphores;
        }
        inline const std::vector<vk::Semaphore>&
        GetRenderCompleteSemaphores() const
        {
            return m_RenderCompleteSemaphores;
        }

      private:
        VulkanSurface                       m_Surface;
        VulkanDevice                        m_Device;
        std::vector<Frame>                  m_Frames;

        vk::SwapchainKHR                    m_SwapChain = VK_NULL_HANDLE;
        [[maybe_unused]] vk::PresentModeKHR m_PresentMode;
        vk::Extent2D                        m_Extent;
        vk::Format                          m_ImageFormat;

        std::vector<vk::Semaphore>          m_ImageAvailableSemaphores;
        std::vector<vk::Semaphore>          m_RenderCompleteSemaphores;

        void                                CreateImageViews();
        void                                CreateCommandBuffers();
        void                                CreateSyncObjects();

        vk::PresentModeKHR                  ChooseSwapPresentMode(
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
        ChooseSwapExtent(const vk::SurfaceCapabilitiesKHR& capabilities);
    };
}; // namespace Vortex
