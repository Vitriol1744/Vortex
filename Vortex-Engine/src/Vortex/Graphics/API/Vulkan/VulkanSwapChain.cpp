//
// Created by Vitriol1744 on 14.09.2022.
//
#include "VulkanSwapChain.hpp"

#include "Vortex/Graphics/API/Vulkan/VulkanRendererAPI.hpp"

namespace Vortex::Graphics
{
    void VulkanSwapChain::CreateSwapChain(VulkanSurface surface, uint32 width, uint32 height)
    {
        VulkanDevice& device = VulkanRendererAPI::GetDevice();
        VkPhysicalDevice physicalDevice = device.GetPhysicalDevice();

        uint32 presentModeCount = 0;
        VkCall_msg(vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface.GetSurface(), &presentModeCount, nullptr),
                   "Vulkan: Failed to Acquire Surface Present Modes!");
        std::vector<VkPresentModeKHR> presentModes(presentModeCount);
        VkCall_msg(vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface.GetSurface(), &presentModeCount, presentModes.data()),
                   "Vulkan: Failed to Acquire Surface Present Modes!");

        presentMode = VK_PRESENT_MODE_FIFO_KHR;
        //TODO: It is probably good strategy to use VK_PRESENT_MODE_FIFO_KHR on mobile devices because of lower energy use
        for (const auto& presentMode : presentModes)
        {
            if (presentMode == VK_PRESENT_MODE_MAILBOX_KHR) this->presentMode = presentMode;
        }

        VkSurfaceFormatKHR surfaceFormat = surface.GetFormat();
        VkSurfaceCapabilitiesKHR surfaceCapabilities = surface.GetCapabilities();

        if (surfaceCapabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) extent = surfaceCapabilities.currentExtent;
        else
        {
            VkExtent2D actualExtent = { static_cast<uint32_t>(width), static_cast<uint32_t>(height) };
            actualExtent.width = std::clamp(actualExtent.width, surfaceCapabilities.minImageExtent.width, surfaceCapabilities.maxImageExtent.width);
            actualExtent.height = std::clamp(actualExtent.height, surfaceCapabilities.minImageExtent.height, surfaceCapabilities.maxImageExtent.height);

            extent = actualExtent;
        }

        VkSwapchainCreateInfoKHR swapChainCreateInfo = {};
        swapChainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        swapChainCreateInfo.pNext = VK_NULL_HANDLE;
        swapChainCreateInfo.flags = 0;
        swapChainCreateInfo.surface = surface.GetSurface();
        swapChainCreateInfo.minImageCount = surfaceCapabilities.maxImageCount >= 3 ? 3 : surfaceCapabilities.minImageCount;
        swapChainCreateInfo.imageFormat = surfaceFormat.format;
        swapChainCreateInfo.imageColorSpace = surfaceFormat.colorSpace;
        swapChainCreateInfo.imageExtent = extent;
        swapChainCreateInfo.imageArrayLayers = 1;
        swapChainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

        uint32 queueFamilies[] = { device.GetGraphicsQueueFamily(), device.GetPresentQueueFamily() };
        if (device.GetGraphicsQueueFamily() != device.GetPresentQueueFamily())
        {
            swapChainCreateInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
            swapChainCreateInfo.queueFamilyIndexCount = 2;
            swapChainCreateInfo.pQueueFamilyIndices = queueFamilies;
        }
        else
        {
            swapChainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
            swapChainCreateInfo.queueFamilyIndexCount = 0;
            swapChainCreateInfo.pQueueFamilyIndices = VK_NULL_HANDLE;
        }
        swapChainCreateInfo.preTransform = surface.GetCapabilities().currentTransform;
        swapChainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        swapChainCreateInfo.presentMode = presentMode;
        swapChainCreateInfo.clipped = VK_TRUE;
        swapChainCreateInfo.oldSwapchain = VK_NULL_HANDLE;

        VkDevice vkDevice = device.GetLogicalDevice();
        VkCall_msg(vkCreateSwapchainKHR(vkDevice, &swapChainCreateInfo, VulkanAllocator::Get().callbacks, &swapChain),
                   "Vulkan: Failed to Create SwapChain!");
        VT_CORE_ASSERT_MSG(swapChain, "Vulkan: Failed to Create SwapChain!");

        uint32 imageCount = 0;
        VkCall_msg(vkGetSwapchainImagesKHR(vkDevice, swapChain, &imageCount, nullptr),
                   "Vulkan: Failed to Acquire Swap Chain Images!");
        images.resize(imageCount);
        VkCall_msg(vkGetSwapchainImagesKHR(vkDevice, swapChain, &imageCount, images.data()),
                   "Vulkan: Failed to Acquire Swap Chain Images!");

        imageFormat = surfaceFormat.format;
    }
    void VulkanSwapChain::CreateImageViews()
    {
        VkDevice device = VulkanRendererAPI::GetDevice().GetLogicalDevice();
        imageViews.resize(images.size());

        for (uint32 i = 0; i < imageViews.size(); i++)
        {
            VkImageViewCreateInfo imageViewCreateInfo = {};
            imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            imageViewCreateInfo.pNext = nullptr;
            imageViewCreateInfo.flags = 0;
            imageViewCreateInfo.image = images[i];
            imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
            imageViewCreateInfo.format = imageFormat;
            imageViewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
            imageViewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
            imageViewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
            imageViewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
            imageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
            imageViewCreateInfo.subresourceRange.levelCount = 1;
            imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
            imageViewCreateInfo.subresourceRange.layerCount = 1;

            VkCall_msg(vkCreateImageView(device, &imageViewCreateInfo, VulkanAllocator::Get().callbacks, &imageViews[i]),
                       "Vulkan: Failed to Create SwapChain Image View!");
        }
    }
}