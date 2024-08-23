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
                           vk::ImageTiling tiling, vk::ImageUsageFlags usage)
        {
            Create(width, height, format, tiling, usage);
        }
        inline ~VulkanImage() { Destroy(); }

        void   Create(u32 width, u32 height, vk::Format format,
                      vk::ImageTiling tiling, vk::ImageUsageFlags usage);
        void   Destroy();

        void   CopyFrom(vk::Buffer buffer, u32 width, u32 height);
        void   TransitionLayout(vk::ImageLayout oldLayout,
                                vk::ImageLayout newLayout);

        inline operator bool() const { return m_Image; }
        inline operator vk::Image() const { return m_Image; }

      private:
        vk::Image      m_Image      = VK_NULL_HANDLE;
        vk::DeviceSize m_Size       = 0;
        VmaAllocation  m_Allocation = VK_NULL_HANDLE;
    };
}; // namespace Vortex
