/*
 * Created by v1tr10l7 on 04.08.2024.
 * Copyright (c) 2024-2024, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include "Vortex/Renderer/API/VertexBuffer.hpp"
#include "Vortex/Renderer/API/Vulkan/VulkanAllocator.hpp"

namespace Vortex
{
    class VT_API VulkanVertexBuffer : public VertexBuffer
    {
      public:
        VulkanVertexBuffer(void* data, usize size);

        virtual ~VulkanVertexBuffer() override;

        inline vk::Buffer GetBuffer() const { return m_VertexBuffer; }

        virtual void SetData(void* data, usize size, usize offset = 0) override
        {
            (void)data;
            (void)size;
            (void)offset;
        }

        inline virtual usize GetSize() const override { return m_Size; }

      private:
        vk::Buffer       m_VertexBuffer       = VK_NULL_HANDLE;
        VmaAllocation    m_MemoryAllocation   = VK_NULL_HANDLE;
        vk::DeviceMemory m_VertexBufferMemory = VK_NULL_HANDLE;
        usize            m_Size               = 0;

        void CopyBuffer(vk::Buffer src, vk::Buffer dest, vk::DeviceSize size);
    };
}; // namespace Vortex
