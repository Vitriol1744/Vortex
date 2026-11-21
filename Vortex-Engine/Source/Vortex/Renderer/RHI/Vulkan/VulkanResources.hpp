/*
 * Created by v1tr10l7 on 15.11.2026.
 * Copyright (c) 2024-2026, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include <Vortex/Renderer/RHI/Vulkan/VulkanCommon.hpp>

#include <mutex>

import Prism.Containers.Vector;

namespace Vortex::Vulkan
{
    class VulkanBuffer final : public RHI::Buffer
    {
      public:
        explicit VulkanBuffer(const RHI::BufferDesc& desc);
        ~VulkanBuffer() override;

        void*             Mapped() override;

        inline vk::Buffer Handle() const { return m_Buffer; }

      private:
        vk::Buffer    m_Buffer     = VK_NULL_HANDLE;
        VmaAllocation m_Allocation = VK_NULL_HANDLE;
        void*         m_Mapped     = nullptr;
    };

    class VulkanTexture final : public RHI::Texture
    {
      public:
        explicit VulkanTexture(const RHI::TextureDesc& desc);
        /// Wraps an image owned by someone else (a swapchain image).
        VulkanTexture(const RHI::TextureDesc& desc, vk::Image image);
        ~VulkanTexture() override;

        inline vk::Image     Handle() const { return m_Image; }
        /// View of every mip and layer (what shaders sample).
        inline vk::ImageView View() const { return m_View; }
        /// Single-subresource 2D attachment view (every aspect). Cached.
        vk::ImageView        View(u32 mip, u32 layer);
        /// One mip as a storage image: 2D, or 2D array over every layer
        /// (cube maps). Cached.
        vk::ImageView        StorageView(u32 mip);
        inline vk::ImageAspectFlags Aspect() const { return m_Aspect; }

      private:
        struct SubresourceView
        {
            u32           Mip   = 0;
            u32           Layer = 0;
            vk::ImageView View  = VK_NULL_HANDLE;
            bool          Storage = false;
        };

        vk::Image               m_Image      = VK_NULL_HANDLE;
        VmaAllocation           m_Allocation = VK_NULL_HANDLE;
        vk::ImageView           m_View       = VK_NULL_HANDLE;
        vk::ImageAspectFlags    m_Aspect{};
        bool                    m_Owned      = true;
        std::mutex              m_ViewMutex;
        Vector<SubresourceView> m_SubresourceViews;

        /// `sampling`: a view shaders read (depth-only for depth/stencil
        /// formats) rather than an attachment view (every aspect).
        vk::ImageView CreateView(vk::ImageViewType type, u32 baseMip,
                                 u32 mipCount, u32 baseLayer, u32 layerCount,
                                 bool sampling);
    };

    class VulkanSampler final : public RHI::Sampler
    {
      public:
        explicit VulkanSampler(const RHI::SamplerDesc& desc);
        ~VulkanSampler() override;

        inline vk::Sampler Handle() const { return m_Sampler; }

      private:
        vk::Sampler m_Sampler = VK_NULL_HANDLE;
    };
} // namespace Vortex::Vulkan
