/*
 * Created by v1tr10l7 on 19.07.2024.
 * Copyright (c) 2024-2024, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include "Vortex/Renderer/API/SwapChain.hpp"
#include "Vortex/Renderer/API/Vulkan/VulkanFramebuffer.hpp"
#include "Vortex/Renderer/API/Vulkan/VulkanImage.hpp"
#include "Vortex/Renderer/API/Vulkan/VulkanSurface.hpp"

namespace Vortex
{
    class VulkanSwapChain : public SwapChain, NonCopyable<VulkanSwapChain>
    {
      public:
        struct PerFrameData
        {
            vk::CommandPool   CommandPool             = VK_NULL_HANDLE;
            vk::CommandBuffer CommandBuffer           = VK_NULL_HANDLE;
            vk::Semaphore     ImageAvailableSemaphore = VK_NULL_HANDLE;
            vk::Semaphore     RenderFinishedSemaphore = VK_NULL_HANDLE;
            vk::Fence         WaitFence               = VK_NULL_HANDLE;
        };

        inline VulkanSwapChain(class Window* window, bool vsync)
        {
            CreateSurface(window);
            Create(vsync);
        }
        virtual ~VulkanSwapChain()
        {
            Destroy();
            DestroySurface();
        }

        void        CreateSurface(Window* windowHandle);
        void        Create(bool vsync = false);

        void        Destroy();
        inline void DestroySurface() { m_Surface.Destroy(); }

        virtual inline WeakRef<Framebuffer> GetFramebuffer() override
        {
            return m_Framebuffer;
        }
        virtual inline void SetVSync(bool vsync) override
        {
            m_VSync        = vsync;
            m_ShouldResize = true;
        }

        void                           BeginFrame();
        virtual void                   Present() override;
        virtual void                   OnResize() override;

        inline const VulkanSurface&    GetSurface() const { return m_Surface; }
        inline const vk::CommandBuffer GetCurrentCommandBuffer() const
        {
            return GetCurrentFrameData().CommandBuffer;
        }
        inline const PerFrameData& GetCurrentFrameData() const
        {
            return m_Frames[m_CurrentFrameIndex];
        }
        inline const std::vector<PerFrameData>& GetPerFrameData() const
        {
            return m_Frames;
        }
        inline const std::vector<VulkanFramebuffer::Frame>& GetFrames() const
        {
            return m_Framebuffer->GetFrames();
        }
        inline u32 GetCurrentFrameIndex() const { return m_CurrentFrameIndex; }
        inline u32 GetCurrentImageIndex() { return m_CurrentImageIndex; }

        inline     operator vk::SwapchainKHR() const { return m_SwapChain; }
        inline const vk::Extent2D& GetExtent() const { return m_Extent; }
        inline vk::Format     GetImageFormat() const { return m_ImageFormat; }
        inline vk::RenderPass GetRenderPass() const
        {
            return m_Framebuffer->GetRenderPass();
        }

      private:
        VulkanSurface                       m_Surface;
        std::vector<PerFrameData>           m_Frames;
        u32                                 m_CurrentFrameIndex = 0;
        u32                                 m_CurrentImageIndex = 0;
        bool                                m_VSync             = false;
        bool                                m_ShouldResize      = false;

        vk::SwapchainKHR                    m_SwapChain = VK_NULL_HANDLE;
        Ref<VulkanFramebuffer>              m_Framebuffer;
        [[maybe_unused]] vk::PresentModeKHR m_PresentMode;
        vk::Extent2D                        m_Extent;
        vk::Format                          m_ImageFormat;

        u32                                 AcquireNextImage();

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
