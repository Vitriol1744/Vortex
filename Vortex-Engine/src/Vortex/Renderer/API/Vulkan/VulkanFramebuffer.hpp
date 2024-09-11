/*
 * Created by v1tr10l7 on 09.09.2024.
 * Copyright (c) 2024-2024, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include "Vortex/Renderer/API/Framebuffer.hpp"
#include "Vortex/Renderer/API/Vulkan/VulkanImage.hpp"

namespace Vortex
{
    class VulkanFramebuffer : public Framebuffer
    {
      public:
        VulkanFramebuffer() = default;
        VulkanFramebuffer(const vk::Extent2D&           extent,
                          const std::vector<vk::Image>& backbuffers,
                          const vk::Format&             format);
        VulkanFramebuffer(const FramebufferSpecification& specs);
        virtual ~VulkanFramebuffer();

        void          Create();
        void          Destroy();

        virtual Vec2u GetSize() const override
        {
            return Vec2u(m_Extent.width, m_Extent.height);
        }
        virtual void OnResize(u32 width, u32 height) override;

        struct Frame
        {
            vk::Image       Image       = VK_NULL_HANDLE;
            vk::ImageView   ImageView   = VK_NULL_HANDLE;
            vk::Framebuffer Framebuffer = VK_NULL_HANDLE;
            vk::Sampler     Sampler     = VK_NULL_HANDLE;
        };

        void        NextImage();
        inline u32  GetCurrentImageIndex() const { return m_CurrentImageIndex; }
        inline void SetCurrentImageIndex(u32 index)
        {
            m_CurrentImageIndex = index;
        }

        inline const vk::Extent2D& GetExtent() const { return m_Extent; }
        inline const Frame&        GetCurrentFrame() const
        {
            return m_Frames[m_CurrentImageIndex];
        }
        inline const std::vector<Frame>& GetFrames() const { return m_Frames; }
        inline const VulkanImage& GetDepthImage() const { return m_DepthImage; }
        inline vk::ImageView      GetDepthImageView() const
        {
            return m_DepthImageView;
        }
        inline const vk::RenderPass GetRenderPass() const
        {
            return m_RenderPass;
        }

      private:
        bool                       m_SwapChainTarget = true;
        vk::Format                 m_ImageFormat;
        vk::Extent2D               m_Extent{};
        std::vector<Frame>         m_Frames{};
        std::vector<VmaAllocation> m_ImageAllocations{};
        VulkanImage                m_DepthImage{};
        vk::ImageView              m_DepthImageView    = VK_NULL_HANDLE;
        vk::RenderPass             m_RenderPass        = VK_NULL_HANDLE;
        u32                        m_CurrentImageIndex = 0;

        void                       CreateImages();
        void                       CreateImageViews();
        void                       CreateDepthBuffer();
        void                       CreateRenderPass();
        void                       CreateFramebuffers();
        void                       CreateSamplers();
    };
}; // namespace Vortex
