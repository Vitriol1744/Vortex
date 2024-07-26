/*
 * Created by v1tr10l7 on 19.07.2024.
 * Copyright (c) 2024-2024, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include "Vortex/Renderer/API/Vulkan/VulkanDevice.hpp"
#include "Vortex/Renderer/API/Vulkan/VulkanSurface.hpp"

namespace Vortex
{
    class VulkanSwapChain
    {
      public:
        struct Frame
        {
            vk::Image         Image                   = VK_NULL_HANDLE;
            vk::ImageView     ImageView               = VK_NULL_HANDLE;
            vk::Framebuffer   Framebuffer             = VK_NULL_HANDLE;
            vk::CommandPool   CommandPool             = VK_NULL_HANDLE;
            vk::CommandBuffer CommandBuffer           = VK_NULL_HANDLE;
            vk::Semaphore     ImageAvailableSemaphore = VK_NULL_HANDLE;
            vk::Semaphore     RenderFinishedSemaphore = VK_NULL_HANDLE;
            vk::Fence         WaitFence               = VK_NULL_HANDLE;
        };

        VulkanSwapChain() = default;

        void Initialize(const VulkanDevice& device) { m_Device = device; }
        void CreateSurface(Window* windowHandle);
        void Create(u32& width, u32& height, bool vsync = false);

        void Destroy();
        inline void                 DestroySurface() { m_Surface.Destroy(); }

        void                        OnResize(u32 width, u32 height);

        inline const VulkanSurface& GetSurface() const { return m_Surface; }
        inline const std::vector<Frame>& GetFrames() const { return m_Frames; }

        inline operator vk::SwapchainKHR() const { return m_SwapChain; }
        inline vk::Extent2D   GetExtent() const { return m_Extent; }
        inline vk::Format     GetImageFormat() const { return m_ImageFormat; }
        inline vk::RenderPass GetRenderPass() const { return m_RenderPass; }

      private:
        VulkanSurface                       m_Surface;
        VulkanDevice                        m_Device;
        std::vector<Frame>                  m_Frames;
        u32                                 m_CurrentFrameIndex  = 0;
        inline static constexpr const u32   MAX_FRAMES_IN_FLIGHT = 2;

        vk::SwapchainKHR                    m_SwapChain = VK_NULL_HANDLE;
        [[maybe_unused]] vk::PresentModeKHR m_PresentMode;
        vk::Extent2D                        m_Extent;
        vk::Format                          m_ImageFormat;
        vk::RenderPass                      m_RenderPass = VK_NULL_HANDLE;

        u32                                 AcquireNextImage();

        void                                CreateImageViews();
        void                                CreateCommandBuffers();
        void                                CreateSyncObjects();
        void                                CreateRenderPass();
        void                                CreateFramebuffers();

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
