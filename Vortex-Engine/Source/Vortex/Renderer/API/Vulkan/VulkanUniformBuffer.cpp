/*
 * Created by v1tr10l7 on 07.08.2024.
 * Copyright (c) 2024-2024, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#include <Vortex/Renderer/API/Vulkan/VulkanUniformBuffer.hpp>
#include <Vortex/Renderer/Renderer.hpp>

namespace Vortex
{
    VulkanUniformBuffer::VulkanUniformBuffer(usize size)
        : m_Size(size)
    {
        u32 maxFramesInFlight = Renderer::Configuration().MaxFramesInFlight;

        m_Buffers.Resize(maxFramesInFlight);
        m_Allocations.Resize(maxFramesInFlight);
        m_MappedAreas.Resize(maxFramesInFlight);
        m_BufferInfos.Resize(maxFramesInFlight);

        vk::BufferCreateInfo bufferInfo{};
        bufferInfo.sType       = vk::StructureType::eBufferCreateInfo;
        bufferInfo.size        = size;
        bufferInfo.usage       = vk::BufferUsageFlagBits::eUniformBuffer;
        bufferInfo.sharingMode = vk::SharingMode::eExclusive;

        for (usize i = 0; i < m_Buffers.Size(); i++)
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
        for (usize i = 0; i < m_Buffers.Size(); i++)
            VulkanAllocator::DestroyBuffer(m_Buffers[i], m_Allocations[i]);
    }

    void VulkanUniformBuffer::SetData(const void* data, usize size,
                                      usize offset)
    {
        VtCoreAssert((size + offset) <= m_Size);
        // TODO(v1tr10l7): it would be better ideaa to only change the current's
        // frame uniform buffer
        for (u32 i = 0; i < m_Allocations.Size(); i++)
        {
            void* dest = VulkanAllocator::MapMemory(m_Allocations[i]);
            Memory::Copy(reinterpret_cast<u8*>(dest) + offset,
                         reinterpret_cast<const u8*>(data) + offset, size);
            VulkanAllocator::UnmapMemory(m_Allocations[i]);
        }
    }

}; // namespace Vortex
