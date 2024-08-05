/*
 * Created by v1tr10l7 on 04.08.2024.
 * Copyright (c) 2024-2024, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#include "vtpch.hpp"

#include "Vortex/Renderer/API/Vulkan/VulkanContext.hpp"
#include "Vortex/Renderer/API/Vulkan/VulkanVertexBuffer.hpp"

namespace Vortex
{
    VulkanVertexBuffer::VulkanVertexBuffer(void* data, usize size)
        : m_Size(size)
    {
        vk::Device       device              = VulkanContext::GetDevice();
        vk::Buffer       stagingBuffer       = VK_NULL_HANDLE;
        vk::DeviceMemory stagingBufferMemory = VK_NULL_HANDLE;
        CreateBuffer(size, vk::BufferUsageFlagBits::eTransferSrc,
                     vk::MemoryPropertyFlagBits::eHostVisible
                         | vk::MemoryPropertyFlagBits::eHostCoherent,
                     stagingBuffer, stagingBufferMemory);

        void* bufferData;
        VkCall(device.mapMemory(stagingBufferMemory, 0, u32(size),
                                vk::MemoryMapFlags(), &bufferData));
        std::memcpy(bufferData, data, size);
        device.unmapMemory(stagingBufferMemory);

        CreateBuffer(size,
                     vk::BufferUsageFlagBits::eTransferDst
                         | vk::BufferUsageFlagBits::eVertexBuffer,
                     vk::MemoryPropertyFlagBits::eDeviceLocal, m_VertexBuffer,
                     m_VertexBufferMemory);
        CopyBuffer(stagingBuffer, m_VertexBuffer, size);
        device.destroyBuffer(stagingBuffer, VK_NULL_HANDLE);
        device.freeMemory(stagingBufferMemory, VK_NULL_HANDLE);
    }

    VulkanVertexBuffer::~VulkanVertexBuffer()
    {
        vk::Device device = VulkanContext::GetDevice();
        device.destroyBuffer(m_VertexBuffer, VK_NULL_HANDLE);
        device.freeMemory(m_VertexBufferMemory, VK_NULL_HANDLE);
    }

    void VulkanVertexBuffer::CreateBuffer(vk::DeviceSize          size,
                                          vk::BufferUsageFlags    usage,
                                          vk::MemoryPropertyFlags properties,
                                          vk::Buffer&             buffer,
                                          vk::DeviceMemory&       bufferMemory)
    {
        vk::BufferCreateInfo createInfo{};
        createInfo.sType       = vk::StructureType::eBufferCreateInfo;
        createInfo.size        = size;
        createInfo.usage       = usage;
        createInfo.sharingMode = vk::SharingMode::eExclusive;

        vk::Device device      = VulkanContext::GetDevice();
        VkCall(device.createBuffer(&createInfo, VK_NULL_HANDLE, &buffer));

        vk::MemoryRequirements memoryRequirements;
        device.getBufferMemoryRequirements(buffer, &memoryRequirements);

        auto memoryType = VulkanContext::GetPhysicalDevice().FindMemoryType(
            memoryRequirements.memoryTypeBits, properties);

        vk::MemoryAllocateInfo allocateInfo{};
        allocateInfo.sType           = vk::StructureType::eMemoryAllocateInfo;
        allocateInfo.allocationSize  = memoryRequirements.size;
        allocateInfo.memoryTypeIndex = memoryType;

        VkCall(device.allocateMemory(&allocateInfo, VK_NULL_HANDLE,
                                     &bufferMemory));
        device.bindBufferMemory(buffer, bufferMemory, 0);
    }

    void VulkanVertexBuffer::CopyBuffer(vk::Buffer src, vk::Buffer dest,
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
