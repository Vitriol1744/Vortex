//
// Created by Vitriol1744 on 18.09.2022.
//
#include "VulkanIndexBuffer.hpp"

#include "Vortex/Graphics/API/Vulkan/VulkanRendererAPI.hpp"

namespace Vortex::Graphics
{
    VulkanIndexBuffer::VulkanIndexBuffer(uint16* data, uint32 size)
            : size(size)
    {
        VkBuffer stagingBuffer;
        VkBufferCreateInfo stagingBufferCreateInfo = {};
        stagingBufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        stagingBufferCreateInfo.size = size;
        stagingBufferCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
        stagingBufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        VmaAllocation stagingBufferAllocation;

        VmaAllocationCreateInfo allocInfo = {};
        allocInfo.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;
        vmaCreateBuffer(VulkanAllocator::Get().allocator, &stagingBufferCreateInfo, &allocInfo, &stagingBuffer, &stagingBufferAllocation, nullptr);

        VkDevice device = VulkanRendererAPI::GetDevice().GetLogicalDevice();
        VkPhysicalDevice physicalDevice = VulkanRendererAPI::GetDevice().GetPhysicalDevice();
        VkAllocationCallbacks* allocator = VulkanAllocator::Get().callbacks;

        void* vertexData;
        vmaMapMemory(VulkanAllocator::Get().allocator, stagingBufferAllocation, &vertexData);
        memcpy(vertexData, data, static_cast<size_t>(size));
        vmaUnmapMemory(VulkanAllocator::Get().allocator, stagingBufferAllocation);

        VkBufferCreateInfo bufferCreateInfo = {};
        bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferCreateInfo.pNext = VK_NULL_HANDLE;
        bufferCreateInfo.flags = 0;
        bufferCreateInfo.size = size;
        bufferCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
        bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        bufferCreateInfo.queueFamilyIndexCount = 0;
        bufferCreateInfo.pQueueFamilyIndices = 0;
        VmaAllocationCreateInfo allocInfo2 = {};
        allocInfo2.usage = VMA_MEMORY_USAGE_GPU_ONLY;
        vmaCreateBuffer(VulkanAllocator::Get().allocator, &bufferCreateInfo, &allocInfo2, &buffer, &allocation, nullptr);

        VkCommandPool commandPool = VulkanRendererAPI::GetCommandPool();
        VkQueue graphicsQueue = VulkanRendererAPI::GetDevice().GetGraphicsQueue();

        VkCommandBufferAllocateInfo commandBufferInfo = {};
        commandBufferInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        commandBufferInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        commandBufferInfo.commandPool = commandPool;
        commandBufferInfo.commandBufferCount = 1;

        VkCommandBuffer commandBuffer;
        vkAllocateCommandBuffers(device, &commandBufferInfo, &commandBuffer);

        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

        vkBeginCommandBuffer(commandBuffer, &beginInfo);

        VkBufferCopy copyRegion = {};
        copyRegion.size = size;
        vkCmdCopyBuffer(commandBuffer, stagingBuffer, buffer, 1, &copyRegion);
        vkEndCommandBuffer(commandBuffer);
        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &commandBuffer;

        vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
        vkQueueWaitIdle(graphicsQueue);
        vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer);
        vmaDestroyBuffer(VulkanAllocator::Get().allocator, stagingBuffer, stagingBufferAllocation);
    }
    VulkanIndexBuffer::~VulkanIndexBuffer()
    {
        VkDevice device = VulkanRendererAPI::GetDevice().GetLogicalDevice();
        VkAllocationCallbacks* allocator = VulkanAllocator::Get().callbacks;

        vkDestroyBuffer(device, buffer, allocator);
    }
}