/*
 * Created by v1tr10l7 on 05.08.2024.
 * Copyright (c) 2024-2024, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include <Vortex/Renderer/API/IndexBuffer.hpp>
#include <Vortex/Renderer/API/Vulkan/VulkanAllocator.hpp>

namespace Vortex
{
    class VT_API VulkanIndexBuffer : public IndexBuffer
    {
      public:
        VulkanIndexBuffer(u32* data, usize size);

        virtual ~VulkanIndexBuffer() override;

        inline vk::Buffer Buffer() const { return m_IndexBuffer; }

        virtual void SetData(u32* data, usize size, usize offset = 0) override
        {
            (void)data;
            (void)size;
            (void)offset;
        }

        inline virtual usize Size() const override { return m_Size; }
        virtual u32 Count() const override { return Size() / sizeof(u32); }

      private:
        vk::Buffer       m_IndexBuffer       = VK_NULL_HANDLE;
        VmaAllocation    m_MemoryAllocation  = VK_NULL_HANDLE;
        vk::DeviceMemory m_IndexBufferMemory = VK_NULL_HANDLE;
        usize            m_Size              = 0;

        void CopyBuffer(vk::Buffer src, vk::Buffer dest, vk::DeviceSize size);
    };
}; // namespace Vortex
