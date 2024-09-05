/*
 * Created by v1tr10l7 on 19.07.2024.
 * Copyright (c) 2024-2024, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include "Vortex/Renderer/API/Vulkan/VulkanDevice.hpp"
#include "Vortex/Renderer/API/Vulkan/VulkanImage.hpp"
#include "Vortex/Renderer/API/Vulkan/VulkanSurface.hpp"

namespace Vortex
{
    class VulkanSwapChain : public NonCopyable<VulkanSwapChain>
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

        void                           CreateSurface(Window* windowHandle);
        void                           Create(bool vsync = false);

        void                           Destroy();
        inline void                    DestroySurface() { m_Surface.Destroy(); }

        inline void                    SetVSync(bool vsync) { m_VSync = vsync; }

        void                           BeginFrame();
        void                           Present();
        void                           OnResize();

        inline const VulkanSurface&    GetSurface() const { return m_Surface; }
        inline const vk::CommandBuffer GetCurrentCommandBuffer() const
        {
            return GetCurrentFrame().CommandBuffer;
        }
        inline const Frame& GetCurrentFrame() const
        {
            return m_Frames[m_CurrentFrameIndex];
        }
        inline const std::vector<Frame>& GetFrames() const { return m_Frames; }
        inline u32 GetCurrentFrameIndex() const { return m_CurrentFrameIndex; }
        inline u32 GetCurrentImageIndex() { return m_CurrentImageIndex; }

        inline     operator vk::SwapchainKHR() const { return m_SwapChain; }
        inline const vk::Extent2D& GetExtent() const { return m_Extent; }
        inline vk::Format     GetImageFormat() const { return m_ImageFormat; }
        inline vk::RenderPass GetRenderPass() const { return m_RenderPass; }

      private:
        VulkanSurface                       m_Surface;
        std::vector<Frame>                  m_Frames;
        u32                                 m_CurrentFrameIndex = 0;
        u32                                 m_CurrentImageIndex = 0;
        bool                                m_VSync             = false;

        vk::SwapchainKHR                    m_SwapChain = VK_NULL_HANDLE;
        [[maybe_unused]] vk::PresentModeKHR m_PresentMode;
        vk::Extent2D                        m_Extent;
        vk::Format                          m_ImageFormat;
        vk::RenderPass                      m_RenderPass = VK_NULL_HANDLE;
        VulkanImage                         m_DepthImage;
        vk::ImageView                       m_DepthImageView = VK_NULL_HANDLE;

        u32                                 AcquireNextImage();

        void                                CreateImageViews();
        void                                CreateCommandBuffers();
        void                                CreateSyncObjects();
        void                                CreateRenderPass();
        void                                CreateFramebuffers();
        void                                CreateDepthBuffer();

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
