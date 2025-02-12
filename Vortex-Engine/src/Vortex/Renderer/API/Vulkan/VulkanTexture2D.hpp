/*
 * Created by v1tr10l7 on 20.08.2024.
 * Copyright (c) 2024-2024, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include "Vortex/Renderer/API/Texture2D.hpp"
#include "Vortex/Renderer/API/Vulkan/VulkanCommon.hpp"
#include "Vortex/Renderer/API/Vulkan/VulkanImage.hpp"

namespace Vortex
{
    class VulkanTexture2D : public Texture2D
    {
      public:
        VulkanTexture2D(PathView path);
        virtual ~VulkanTexture2D();

        virtual u32               GetMipLevelCount() override { return 0; }

        inline const VulkanImage& GetImage() const { return m_TextureImage; }
        inline vk::ImageView      GetImageView() const { return m_TextureView; }
        inline vk::Sampler GetSampler() const { return m_TextureSampler; }

        inline const vk::DescriptorImageInfo& GetImageInfo() const
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
