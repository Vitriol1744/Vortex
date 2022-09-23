//
// Created by Vitriol1744 on 19.09.2022.
//
#include "VulkanUniformBuffer.hpp"

#include "Vortex/Graphics/API/Vulkan/VulkanRendererAPI.hpp"

namespace Vortex::Graphics
{
    VulkanUniformBuffer::VulkanUniformBuffer(size_t size)
    {
        //TODO: Use VulkanRendererAPI::GetMaxFramesInFlight()
        buffers.resize(2);
        //TODO: and here
        for (size_t i = 0; i < 2; i++)
        {
            VkBufferCreateInfo bufferCreateInfo = {};
            bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
            bufferCreateInfo.size = size;
            bufferCreateInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
            bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

            VmaAllocationCreateInfo allocInfo = {};
            allocInfo.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;
            vmaCreateBuffer(VulkanAllocator::Get().allocator, &bufferCreateInfo, &allocInfo, &buffers[i], &bufferAllocations[i], nullptr);
        }

        this->size = size;
    }
    VulkanUniformBuffer::~VulkanUniformBuffer()
    {
        for (size_t i = 0; i < 2; i++)
            vmaDestroyBuffer(VulkanAllocator::Get().allocator, buffers[i], bufferAllocations[i]);
        VkDevice device = VulkanRendererAPI::GetDevice().GetLogicalDevice();
    }

    void VulkanUniformBuffer::SetData(void* data)
    {
        uint32 currentFrameIndex = VulkanRendererAPI::GetCurrentFrameIndex();
        void* mapData;
        vmaMapMemory(VulkanAllocator::Get().allocator, bufferAllocations[currentFrameIndex], &mapData);
        memcpy(mapData, &data, size);
        vmaUnmapMemory(VulkanAllocator::Get().allocator, bufferAllocations[currentFrameIndex]);
    }
}