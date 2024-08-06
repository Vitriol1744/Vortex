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
    VulkanIndexBuffer::VulkanIndexBuffer(void* data, usize size)
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
        vk::Device                device = VulkanContext::GetDevice();
        // TODO(v1tr10l7): TEMPORARY:
        vk::CommandPool           commandPool{};
        vk::CommandPoolCreateInfo poolCreateInfo{};
        poolCreateInfo.sType = vk::StructureType::eCommandPoolCreateInfo;
        poolCreateInfo.pNext = VK_NULL_HANDLE;
        poolCreateInfo.flags
            = vk::CommandPoolCreateFlagBits::eResetCommandBuffer;
        poolCreateInfo.queueFamilyIndex = VulkanContext::GetPhysicalDevice()
                                              .GetQueueFamilyIndices()
                                              .Transfer.value();
        VkCall(device.createCommandPool(&poolCreateInfo, VK_NULL_HANDLE,
                                        &commandPool));

        vk::CommandBufferAllocateInfo allocInfo{};
        allocInfo.sType       = vk::StructureType::eCommandBufferAllocateInfo;
        allocInfo.level       = vk::CommandBufferLevel::ePrimary;
        allocInfo.commandPool = commandPool;
        allocInfo.commandBufferCount = 1;

        vk::CommandBuffer commandBuffer{};
        VkCall(device.allocateCommandBuffers(&allocInfo, &commandBuffer));

        vk::CommandBufferBeginInfo beginInfo{};
        beginInfo.sType = vk::StructureType::eCommandBufferBeginInfo;
        beginInfo.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit;
        VkCall(commandBuffer.begin(&beginInfo));

        vk::BufferCopy copyRegion{};
        copyRegion.srcOffset = 0;
        copyRegion.dstOffset = 0;
        copyRegion.size      = size;
        commandBuffer.copyBuffer(src, dest, 1, &copyRegion);
        commandBuffer.end();

        vk::SubmitInfo submit{};
        submit.sType              = vk::StructureType::eSubmitInfo;
        submit.commandBufferCount = 1;
        submit.pCommandBuffers    = &commandBuffer;

        vk::Queue transferQueue = VulkanContext::GetDevice().GetTransferQueue();
        VkCall(transferQueue.submit(1, &submit, VK_NULL_HANDLE));
        transferQueue.waitIdle();

        device.freeCommandBuffers(commandPool, 1, &commandBuffer);
        device.destroyCommandPool(commandPool);
    }
}; // namespace Vortex