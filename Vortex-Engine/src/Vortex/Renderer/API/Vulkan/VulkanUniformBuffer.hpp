/*
 * Created by v1tr10l7 on 07.08.2024.
 * Copyright (c) 2024-2024, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include "Vortex/Renderer/API/UniformBuffer.hpp"
#include "Vortex/Renderer/API/Vulkan/VulkanCommon.hpp"

namespace Vortex
{
    class VulkanUniformBuffer : public UniformBuffer
    {
      public:
        VulkanUniformBuffer(usize size);
        virtual ~VulkanUniformBuffer();

        virtual void SetData(const void* data, usize size,
                             usize offset = 0) override;

      private:
        // TODO(v1tr10l7): should be private
      public:
        usize                                 m_Size = 0;
        std::vector<vk::Buffer>               m_Buffers{};
        std::vector<VmaAllocation>            m_Allocations{};
        std::vector<void*>                    m_MappedAreas;

        std::vector<vk::DescriptorBufferInfo> m_BufferInfos{};
    };
}; // namespace Vortex
