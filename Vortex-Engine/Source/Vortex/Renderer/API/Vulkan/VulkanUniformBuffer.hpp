/*
 * Created by v1tr10l7 on 07.08.2024.
 * Copyright (c) 2024-2024, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include <Vortex/Renderer/API/UniformBuffer.hpp>
#include <Vortex/Renderer/API/Vulkan/VulkanAllocator.hpp>

namespace Vortex
{
    class VulkanUniformBuffer : public UniformBuffer
    {
      public:
        VulkanUniformBuffer(usize size);
        virtual ~VulkanUniformBuffer();

        virtual void SetData(const void* data, usize size,
                             usize offset = 0) override;

        inline const Vector<vk::DescriptorBufferInfo>&
        BufferInfos() const
        {
            return m_BufferInfos;
        }

      private:
        usize                                 m_Size = 0;
        Vector<vk::Buffer>               m_Buffers{};
        Vector<VmaAllocation>            m_Allocations{};
        Vector<void*>                    m_MappedAreas;

        Vector<vk::DescriptorBufferInfo> m_BufferInfos{};
    };
}; // namespace Vortex
