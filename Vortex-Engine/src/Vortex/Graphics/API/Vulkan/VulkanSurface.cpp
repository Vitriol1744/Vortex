//
// Created by Vitriol1744 on 14.09.2022.
//
#include "VulkanSurface.hpp"

#include "Vortex/Graphics/API/Vulkan/VulkanRendererAPI.hpp"

namespace Vortex::Graphics
{
    static uint32 surfaceCount = 0;

    void VulkanSurface::Create(GLFWwindow* windowHandle)
    {
        VkInstance vulkanInstance = VulkanRendererAPI::GetVulkanInstance().instance;
        VkAllocationCallbacks* allocator = VulkanAllocator::Get().callbacks;
        glfwCreateWindowSurface(vulkanInstance, windowHandle, allocator, &surface);

        VulkanDevice& device = VulkanRendererAPI::GetDevice();
        if (surfaceCount == 0)
        {
            device.Create(surface);
            VulkanRendererAPI::Get()->CreateCommandPool();
            VulkanRendererAPI::Get()->CreateCommandBuffers();
            VulkanRendererAPI::Get()->CreateSyncObjects();
            VulkanAllocator::InitializeVMA();
        }
        surfaceCount++;

        VkPhysicalDevice physicalDevice = device.GetPhysicalDevice();

        uint32 formatCount = 0;
        VkCall_msg(vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, nullptr),
                   "Failed to Acquire Surface Formats!");
        std::vector<VkSurfaceFormatKHR> surfaceFormats(formatCount);
        VkCall_msg(vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, surfaceFormats.data()),
                   "Failed to Acquire Surface Formats!");
        VT_CORE_ASSERT_MSG(formatCount, "Could not find any surface formats!");

        format = *(surfaceFormats.data());
        for (const auto& surfaceFormat : surfaceFormats)
        {
            if (surfaceFormat.format == VK_FORMAT_B8G8R8A8_SRGB && surfaceFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
                format = surfaceFormat;
            if (surfaceFormat.format == VK_FORMAT_B8G8R8A8_UNORM && surfaceFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
            {
                format = surfaceFormat;
                break;
            }
        }
    }
    void VulkanSurface::Destroy()
    {
        surfaceCount--;
        if (surfaceCount == 0)
        {
            VulkanRendererAPI::Get()->DestroySyncObjects();
            VulkanRendererAPI::Get()->DestroyCommandPool();
            VulkanDevice& device = VulkanRendererAPI::GetDevice();
            device.Destroy();
        }

        VkInstance vulkanInstance = VulkanRendererAPI::GetVulkanInstance().instance;
        vkDestroySurfaceKHR(vulkanInstance, surface, nullptr);
    }
}
