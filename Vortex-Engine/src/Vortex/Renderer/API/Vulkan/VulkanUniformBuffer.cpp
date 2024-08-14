/*
 * Created by v1tr10l7 on 07.08.2024.
 * Copyright (c) 2024-2024, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#include "vtpch.hpp"

#include "Vortex/Renderer/API/Vulkan/VulkanContext.hpp"
#include "Vortex/Renderer/API/Vulkan/VulkanUniformBuffer.hpp"

namespace Vortex
{
    // TODO(v1tr10l7): hardcoded here for now

    VulkanUniformBuffer::VulkanUniformBuffer(usize size)
        : m_Size(size)
    {
        m_Buffers.resize(VT_MAX_FRAMES_IN_FLIGHT);
        m_Allocations.resize(VT_MAX_FRAMES_IN_FLIGHT);
        m_MappedAreas.resize(VT_MAX_FRAMES_IN_FLIGHT);
        m_BufferInfos.resize(VT_MAX_FRAMES_IN_FLIGHT);

        vk::BufferCreateInfo bufferInfo{};
        bufferInfo.sType       = vk::StructureType::eBufferCreateInfo;
        bufferInfo.size        = 1024;
        bufferInfo.usage       = vk::BufferUsageFlagBits::eUniformBuffer;
        bufferInfo.sharingMode = vk::SharingMode::eExclusive;

        for (usize i = 0; i < VT_MAX_FRAMES_IN_FLIGHT; i++)
        {
            vk::DescriptorBufferInfo& descriptorBufferInfo = m_BufferInfos[i];
            m_Allocations[i] = VulkanAllocator::AllocateBuffer(
                bufferInfo, VMA_MEMORY_USAGE_CPU_TO_GPU, m_Buffers[i]);

            descriptorBufferInfo.buffer = m_Buffers[i];
            descriptorBufferInfo.offset = 0;
            descriptorBufferInfo.range  = size;
        }
    }
    VulkanUniformBuffer::~VulkanUniformBuffer()
    {
        for (usize i = 0; i < VT_MAX_FRAMES_IN_FLIGHT; i++)
            VulkanAllocator::DestroyBuffer(m_Buffers[i], m_Allocations[i]);
    }

    void VulkanUniformBuffer::SetData(const void* data, usize size,
                                      usize offset)
    {
        (void)data;
        (void)size;
        (void)offset;
    }

}; // namespace Vortex
