/*
 * Created by v1tr10l7 on 05.08.2024.
 * Copyright (c) 2024-2024, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#include "vtpch.hpp"

#include "Vortex/Renderer/API/Vulkan/VulkanAllocator.hpp"
#include "Vortex/Renderer/API/Vulkan/VulkanContext.hpp"

namespace Vortex::VulkanAllocator
{
    static VmaAllocator s_Allocator = VK_NULL_HANDLE;

    void                Initialize()
    {
        VmaAllocatorCreateInfo createInfo{};
        createInfo.vulkanApiVersion
            = VulkanContext::GetInstance().GetUsedApiVersion();
        createInfo.physicalDevice
            = vk::PhysicalDevice(VulkanContext::GetPhysicalDevice());
        createInfo.device   = vk::Device(VulkanContext::GetDevice());
        createInfo.instance = vk::Instance(VulkanContext::GetInstance());

        vmaCreateAllocator(&createInfo, &s_Allocator);
    }
    void          Shutdown() { vmaDestroyAllocator(s_Allocator); }

    VmaAllocation AllocateBuffer(vk::BufferCreateInfo bufferInfo,
                                 VmaMemoryUsage usage, vk::Buffer& outBuffer)
    {
        VmaAllocationCreateInfo allocInfo{};
        allocInfo.usage = usage;

        VmaAllocation allocation{};
        vmaCreateBuffer(s_Allocator, (VkBufferCreateInfo*)&bufferInfo,
                        &allocInfo, (VkBuffer*)&outBuffer, &allocation,
                        VK_NULL_HANDLE);
        VtCoreAssert(allocation != VK_NULL_HANDLE);

        return allocation;
    }
    void DestroyBuffer(vk::Buffer buffer, VmaAllocation allocation)
    {
        vmaDestroyBuffer(s_Allocator, buffer, allocation);
    }

    void* MapMemory(VmaAllocation allocation)
    {
        void* mappedMemory = nullptr;
        vmaMapMemory(s_Allocator, allocation, &mappedMemory);

        return mappedMemory;
    }
    void UnmapMemory(VmaAllocation allocation)
    {
        vmaUnmapMemory(s_Allocator, allocation);
    }

}; // namespace Vortex::VulkanAllocator
