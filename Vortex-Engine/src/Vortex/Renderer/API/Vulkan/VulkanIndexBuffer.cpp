/*
 * Created by v1tr10l7 on 05.08.2024.
 * Copyright (c) 2024-2024, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#include "vtpch.hpp"

#include "Vortex/Renderer/API/Vulkan/VulkanAllocator.hpp"
#include "Vortex/Renderer/API/Vulkan/VulkanContext.hpp"
#include "Vortex/Renderer/API/Vulkan/VulkanIndexBuffer.hpp"

namespace Vortex
{
    VulkanIndexBuffer::VulkanIndexBuffer(u32* data, usize size)
        : m_Size(size)
    {
        vk::BufferCreateInfo stagingInfo{};
        stagingInfo.sType       = vk::StructureType::eBufferCreateInfo;
        stagingInfo.size        = size;
        stagingInfo.usage       = vk::BufferUsageFlagBits::eTransferSrc;
        stagingInfo.sharingMode = vk::SharingMode::eExclusive;

        vk::Buffer    stagingBuffer{};
        VmaAllocation stagingAllocation = VulkanAllocator::AllocateBuffer(
            stagingInfo, VMA_MEMORY_USAGE_CPU_TO_GPU, stagingBuffer);

        u8* dest = VulkanAllocator::MapMemory<u8*>(stagingAllocation);
        std::memcpy(dest, data, size);
        VulkanAllocator::UnmapMemory(stagingAllocation);

        vk::BufferCreateInfo indexBufferInfo{};
        indexBufferInfo.sType = vk::StructureType::eBufferCreateInfo;
        indexBufferInfo.size  = size;
        indexBufferInfo.usage = vk::BufferUsageFlagBits::eTransferDst
                              | vk::BufferUsageFlagBits::eIndexBuffer;
        indexBufferInfo.sharingMode = vk::SharingMode::eExclusive;

        m_MemoryAllocation          = VulkanAllocator::AllocateBuffer(
            indexBufferInfo, VMA_MEMORY_USAGE_GPU_ONLY, m_IndexBuffer);
        CopyBuffer(stagingBuffer, m_IndexBuffer, size);
        VulkanAllocator::DestroyBuffer(stagingBuffer, stagingAllocation);
    }

    VulkanIndexBuffer::~VulkanIndexBuffer()
    {
        VulkanAllocator::DestroyBuffer(m_IndexBuffer, m_MemoryAllocation);
    }

    void VulkanIndexBuffer::CopyBuffer(vk::Buffer src, vk::Buffer dest,
                                       vk::DeviceSize size)
    {
        const VulkanDevice& device        = VulkanContext::GetDevice();
        vk::CommandBuffer   commandBuffer = device.BeginTransferCommand();

        vk::BufferCopy      copyRegion{};
        copyRegion.srcOffset = 0;
        copyRegion.dstOffset = 0;
        copyRegion.size      = size;
        commandBuffer.copyBuffer(src, dest, 1, &copyRegion);

        device.EndTransferCommand(commandBuffer);
    }
}; // namespace Vortex
