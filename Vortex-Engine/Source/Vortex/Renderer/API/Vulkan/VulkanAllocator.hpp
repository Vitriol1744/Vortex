/*
 * Created by v1tr10l7 on 05.08.2024.
 * Copyright (c) 2024-2024, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include <vk_mem_alloc.h>

namespace Vortex
{
    namespace VulkanAllocator
    {
        void          Initialize();
        void          Shutdown();

        VmaAllocation AllocateImage(vk::ImageCreateInfo imageInfo,
                                    VmaMemoryUsage usage, vk::Image& outImage,
                                    vk::DeviceSize& outAllocateSize);
        VmaAllocation AllocateBuffer(vk::BufferCreateInfo bufferInfo,
                                     VmaMemoryUsage       usage,
                                     vk::Buffer&          outBuffer);

        void          DestroyImage(vk::Image image, VmaAllocation allocation);
        void       DestroyBuffer(vk::Buffer buffer, VmaAllocation allocation);

        vk::Result BindImageMemory(VmaAllocation allocation, vk::Image image);

        void*      MapMemory(VmaAllocation allocation);
        void       UnmapMemory(VmaAllocation allocation);

        template <typename T>
        T MapMemory(VmaAllocation allocation)
        {
            return reinterpret_cast<T>(MapMemory(allocation));
        }

    }; // namespace VulkanAllocator
};     // namespace Vortex
