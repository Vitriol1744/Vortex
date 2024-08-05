/*
 * Created by v1tr10l7 on 05.08.2024.
 * Copyright (c) 2024-2024, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include "Vortex/Renderer/API/Vulkan/VulkanCommon.hpp"

#include <vk_mem_alloc.h>

namespace Vortex
{
    namespace VulkanAllocator
    {
        void          Initialize();
        void          Shutdown();

        VmaAllocation AllocateBuffer(vk::BufferCreateInfo bufferInfo,
                                     VmaMemoryUsage       usage,
                                     vk::Buffer&          outBuffer);
        void  DestroyBuffer(vk::Buffer buffer, VmaAllocation allocation);

        void* MapMemory(VmaAllocation allocation);
        void  UnmapMemory(VmaAllocation allocation);

        template <typename T>
        T MapMemory(VmaAllocation allocation)
        {
            return reinterpret_cast<T>(MapMemory(allocation));
        }

    }; // namespace VulkanAllocator
};     // namespace Vortex
