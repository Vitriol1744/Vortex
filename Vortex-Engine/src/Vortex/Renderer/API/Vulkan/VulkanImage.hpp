/*
 * Created by v1tr10l7 on 23.08.2024.
 * Copyright (c) 2024-2024, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include "Vortex/Core/NonCopyable.hpp"
#include "Vortex/Renderer/API/Vulkan/VulkanCommon.hpp"

namespace Vortex
{
    class VulkanImage : public NonCopyable<VulkanImage>
    {
      public:
        VulkanImage() = default;
        inline VulkanImage(u32 width, u32 height, vk::Format format,
                           vk::ImageTiling tiling, vk::ImageUsageFlags usage,
                           u32 mipLevels = 1)
        {
            Create(width, height, format, tiling, usage, mipLevels);
        }
        inline ~VulkanImage() { Destroy(); }

        void   Create(u32 width, u32 height, vk::Format format,
                      vk::ImageTiling tiling, vk::ImageUsageFlags usage,
                      u32 mipLevels = 1);
        void   Destroy();

        void   CopyFrom(vk::Buffer buffer, u32 width, u32 height);
        void   TransitionLayout(vk::ImageLayout oldLayout,
                                vk::ImageLayout newLayout, u32 mipLevels = 1);

        void   GenerateMipMaps(i32 width, i32 height, u32 mipLevels);

        inline operator bool() const { return m_Image; }
        inline operator vk::Image() const { return m_Image; }

      private:
        vk::Image      m_Image      = VK_NULL_HANDLE;
        vk::DeviceSize m_Size       = 0;
        VmaAllocation  m_Allocation = VK_NULL_HANDLE;
        u32            m_MipLevels  = 1;
    };
}; // namespace Vortex
