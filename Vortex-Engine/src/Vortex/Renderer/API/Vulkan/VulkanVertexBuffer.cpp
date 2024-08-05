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
        vk::BufferCreateInfo createInfo{};
        createInfo.sType       = vk::StructureType::eBufferCreateInfo;
        createInfo.size        = size;
        createInfo.usage       = vk::BufferUsageFlagBits::eVertexBuffer;
        createInfo.sharingMode = vk::SharingMode::eExclusive;

        vk::Device device      = VulkanContext::GetDevice();
        VkCall(
            device.createBuffer(&createInfo, VK_NULL_HANDLE, &m_VertexBuffer));

        vk::MemoryRequirements memoryRequirements;
        device.getBufferMemoryRequirements(m_VertexBuffer, &memoryRequirements);

        auto memoryType = VulkanContext::GetPhysicalDevice().FindMemoryType(
            memoryRequirements.memoryTypeBits,
            vk::MemoryPropertyFlagBits::eHostVisible
                | vk::MemoryPropertyFlagBits::eHostCoherent);

        vk::MemoryAllocateInfo allocateInfo{};
        allocateInfo.sType           = vk::StructureType::eMemoryAllocateInfo;
        allocateInfo.allocationSize  = memoryRequirements.size;
        allocateInfo.memoryTypeIndex = memoryType;

        VkCall(device.allocateMemory(&allocateInfo, VK_NULL_HANDLE,
                                     &m_VertexBufferMemory));
        device.bindBufferMemory(m_VertexBuffer, m_VertexBufferMemory, 0);
        void* bufferData;
        VkCall(device.mapMemory(m_VertexBufferMemory, 0, u32(m_Size),
                                vk::MemoryMapFlags(), &bufferData));
        std::memcpy(bufferData, data, m_Size);
        device.unmapMemory(m_VertexBufferMemory);
    }

    VulkanVertexBuffer::~VulkanVertexBuffer()
    {
        vk::Device device = VulkanContext::GetDevice();
        device.destroyBuffer(m_VertexBuffer, VK_NULL_HANDLE);
        device.freeMemory(m_VertexBufferMemory, VK_NULL_HANDLE);
    }
}; // namespace Vortex
