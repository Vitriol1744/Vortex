//
// Created by Vitriol1744 on 17.09.2022.
//
#include "Vortex/vtpch.hpp"
#define VMA_IMPLEMENTATION
#define VMA_VULKAN_VERSION 1002000
#include "vk_mem_alloc.h"
#include "VulkanAllocator.hpp"

#include "Vortex/Graphics/API/Vulkan/VulkanRendererAPI.hpp"

namespace Vortex::Graphics
{
    void VulkanAllocator::InitializeVMA()
    {
        VulkanDevice& device = VulkanRendererAPI::GetDevice();

        VmaAllocatorCreateInfo createInfo = {};
        createInfo.vulkanApiVersion = VK_API_VERSION_1_2;
        createInfo.physicalDevice = device.GetPhysicalDevice();
        createInfo.device = device.GetLogicalDevice();
        createInfo.instance = VulkanRendererAPI::GetVulkanInstance().instance;

        vmaCreateAllocator(&createInfo, &Get().allocator);
    }

    VulkanAllocator::VulkanAllocator()
    {
        //TODO: Create Callbacks!
    }
}