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
        virtual ~VulkanFramebuffer();

        struct Frame
        {
            vk::Image       Image       = VK_NULL_HANDLE;
            vk::ImageView   ImageView   = VK_NULL_HANDLE;
            vk::Framebuffer Framebuffer = VK_NULL_HANDLE;
        };

        inline const vk::Extent2D&       GetExtent() const { return m_Extent; }
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
        vk::Format         m_ImageFormat;
        vk::Extent2D       m_Extent{};
        std::vector<Frame> m_Frames{};
        VulkanImage        m_DepthImage{};
        vk::ImageView      m_DepthImageView = VK_NULL_HANDLE;
        vk::RenderPass     m_RenderPass     = VK_NULL_HANDLE;

        void               CreateImageViews();
        void               CreateDepthBuffer();
        void               CreateRenderPass();
        void               CreateFramebuffers();
    };
}; // namespace Vortex
