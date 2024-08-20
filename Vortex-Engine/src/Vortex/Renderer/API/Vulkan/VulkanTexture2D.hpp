/*
 * Created by v1tr10l7 on 20.08.2024.
 * Copyright (c) 2024-2024, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include "Vortex/Renderer/API/Texture2D.hpp"
#include "Vortex/Renderer/API/Vulkan/VulkanCommon.hpp"

namespace Vortex
{
    class VulkanTexture2D : public Texture2D
    {
      public:
        VulkanTexture2D(PathView path);
        virtual ~VulkanTexture2D();

        virtual u32 GetMipLevelCount() override { return 0; }

        auto&       GetImageInfo() const { return m_ImageInfo; }

      private:
        vk::Image               m_TextureImage;
        vk::ImageView           m_TextureView;
        vk::DeviceSize          m_Size;
        vk::DescriptorImageInfo m_ImageInfo;
        vk::Sampler             m_TextureSampler = VK_NULL_HANDLE;
        VmaAllocation           m_Allocation     = VK_NULL_HANDLE;

        void TransitionImageLayout(vk::Image image, vk::Format format,
                                   vk::ImageLayout oldLayout,
                                   vk::ImageLayout newLayout);
        void CopyBufferToImage(vk::Buffer buffer, vk::Image image, u32 width,
                               u32 height);
    };
}; // namespace Vortex
