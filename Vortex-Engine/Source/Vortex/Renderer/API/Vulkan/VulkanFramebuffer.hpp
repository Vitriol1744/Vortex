/*
 * Created by v1tr10l7 on 09.09.2024.
 * Copyright (c) 2024-2024, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include <Prism/Containers/Vector.hpp>
#include <Vortex/Renderer/API/Framebuffer.hpp>
#include <Vortex/Renderer/API/Vulkan/VulkanImage.hpp>

namespace Vortex
{
    class VulkanFramebuffer : public Framebuffer
    {
      public:
        VulkanFramebuffer() = default;
        VulkanFramebuffer(const vk::Extent2D&      extent,
                          const Vector<vk::Image>& backbuffers,
                          const vk::Format&        format);
        VulkanFramebuffer(const FramebufferSpecification& specs);
        virtual ~VulkanFramebuffer();

        void                Create();
        void                Destroy();

        virtual Math::Vec2u Size() const override
        {
            return Math::Vec2u(m_Extent.width, m_Extent.height);
        }
        virtual ::Ref<Image2D> Image(u32 index = 0) const override
        {
            VtCoreAssertMsg(index < m_Frames.Size(),
                            "Image index out of bounds");
            return m_Frames[index].Image;
        }

        virtual void OnResize(u32 width, u32 height) override;

        struct Frame
        {
            ::Ref<VulkanImage> Image       = nullptr;
            vk::ImageView      ImageView   = VK_NULL_HANDLE;
            vk::Framebuffer    Framebuffer = VK_NULL_HANDLE;
        };

        void        NextImage();
        inline u32  CurrentImageIndex() const { return m_CurrentImageIndex; }
        inline void SetCurrentImageIndex(u32 index)
        {
            m_CurrentImageIndex = index;
        }

        inline const vk::Extent2D& Extent() const { return m_Extent; }
        inline vk::Framebuffer     GetVulkanFramebuffer() const
        {
            return m_Frames[m_CurrentImageIndex].Framebuffer;
        }
        // inline const Frame&        CurrentFrame() const
        // {
        //     return m_Frames[m_CurrentImageIndex];
        // }
        inline vk::Sampler Sampler() const { return m_Frames[0].Image->Sampler(); }
        inline vk::ImageView ImageView() const { return m_Frames[0].ImageView; }
        inline const Vector<Frame>& Frames() const { return m_Frames; }
        inline const VulkanImage&   DepthImage() const { return m_DepthImage; }
        inline vk::ImageView DepthImageView() const { return m_DepthImageView; }
        inline const vk::RenderPass RenderPass() const { return m_RenderPass; }

      private:
        bool                  m_SwapChainTarget = true;
        vk::Format            m_ImageFormat;
        vk::Extent2D          m_Extent{};
        Vector<Frame>         m_Frames{};
        Vector<VmaAllocation> m_ImageAllocations{};
        VulkanImage           m_DepthImage{};
        vk::ImageView         m_DepthImageView    = VK_NULL_HANDLE;
        vk::RenderPass        m_RenderPass        = VK_NULL_HANDLE;
        u32                   m_CurrentImageIndex = 0;

        void                  CreateImages();
        void                  CreateImageViews();
        void                  CreateDepthBuffer();
        void                  CreateRenderPass();
        void                  CreateFramebuffers();
        void                  CreateSamplers();

        friend class VulkanSwapChain;
    };
}; // namespace Vortex
