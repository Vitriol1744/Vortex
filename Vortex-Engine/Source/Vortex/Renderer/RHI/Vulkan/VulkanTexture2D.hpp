/*
 * Created by v1tr10l7 on 20.08.2024.
 * Copyright (c) 2024-2024, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include <Vortex/Renderer/RHI/Texture2D.hpp>
#include <Vortex/Renderer/RHI/Vulkan/VulkanCommon.hpp>
#include <Vortex/Renderer/RHI/Vulkan/VulkanImage.hpp>

namespace Vortex
{
    class VT_RENDERER_RHI_VULKAN_API VulkanTexture2D : public Texture2D
    {
      public:
        VulkanTexture2D(ImageView image);
        virtual ~VulkanTexture2D();

        virtual u32               MipLevelCount() override { return 0; }

        inline const VulkanImage& Image() const { return m_TextureImage; }
        inline vk::ImageView      ImageView() const { return m_TextureView; }
        inline vk::Sampler        Sampler() const { return m_TextureSampler; }

        inline const vk::DescriptorImageInfo& ImageInfo() const
        {
            return m_ImageInfo;
        }

      private:
        VulkanImage             m_TextureImage;
        vk::ImageView           m_TextureView;
        vk::Sampler             m_TextureSampler = VK_NULL_HANDLE;

        vk::DeviceSize          m_Size;
        vk::DescriptorImageInfo m_ImageInfo;
    };
}; // namespace Vortex
