/*
 * Created by v1tr10l7 on 04.08.2024.
 * Copyright (c) 2024-2024, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#include "vtpch.hpp"

#include "Vortex/Renderer/API/Vulkan/VulkanAllocator.hpp"
#include "Vortex/Renderer/API/Vulkan/VulkanRenderer.hpp"
#include "Vortex/Renderer/API/Vulkan/VulkanVertexBuffer.hpp"

namespace Vortex
{
    VulkanVertexBuffer::VulkanVertexBuffer(void* data, usize size)
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

        vk::BufferCreateInfo vertexBufferInfo{};
        vertexBufferInfo.sType = vk::StructureType::eBufferCreateInfo;
        vertexBufferInfo.size  = size;
        vertexBufferInfo.usage = vk::BufferUsageFlagBits::eTransferDst
                               | vk::BufferUsageFlagBits::eVertexBuffer;
        vertexBufferInfo.sharingMode = vk::SharingMode::eExclusive;

        m_MemoryAllocation           = VulkanAllocator::AllocateBuffer(
            vertexBufferInfo, VMA_MEMORY_USAGE_GPU_ONLY, m_VertexBuffer);
        CopyBuffer(stagingBuffer, m_VertexBuffer, size);
        VulkanAllocator::DestroyBuffer(stagingBuffer, stagingAllocation);
    }

    VulkanVertexBuffer::~VulkanVertexBuffer()
    {
        VulkanAllocator::DestroyBuffer(m_VertexBuffer, m_MemoryAllocation);
    }

    void VulkanVertexBuffer::CopyBuffer(vk::Buffer src, vk::Buffer dest,
                                        vk::DeviceSize size)
    {
        const VulkanDevice& device        = VulkanRenderer::GetDevice();
        vk::CommandBuffer   commandBuffer = device.BeginTransferCommand();

        vk::BufferCopy      copyRegion{};
        copyRegion.srcOffset = 0;
        copyRegion.dstOffset = 0;
        copyRegion.size      = size;
        commandBuffer.copyBuffer(src, dest, 1, &copyRegion);

        device.EndTransferCommand(commandBuffer);
    }
}; // namespace Vortex
