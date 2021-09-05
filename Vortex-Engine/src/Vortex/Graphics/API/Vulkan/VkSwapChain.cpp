//
// Created by vitriol1744 on 02.09.2021.
//
#include "VkSwapChain.hpp"

#include "Vortex/Graphics/API/Vulkan/VkRendererAPI.hpp"

namespace Vortex::Graphics
{
    VkSwapChain::~VkSwapChain()
    {
        vk::Device& device = VkRendererAPI::GetDevice().GetLogicalDevice();

        for (auto& imageView : imageViews)
        {
            device.destroyImageView(imageView, VkAllocator::Get().callbacks);
        }
        device.destroySwapchainKHR(swapChain, VkAllocator::Get().callbacks);
    }

    void VkSwapChain::Initialize(const vk::Extent2D extent, const VkSurface& surface)
    {
        this->extent = extent;

        CreateSwapChain(surface);
        CreateImageViews();
    }

    void VkSwapChain::CreateSwapChain(const VkSurface& surface)
    {
        VkDevice& device = VkRendererAPI::GetDevice();
        vk::PhysicalDevice& physicalDevice = device.GetPhysicalDevice();

        uint32 presentModeCount = 0;
        VkCall(physicalDevice.getSurfacePresentModesKHR(surface.GetSurface(), &presentModeCount, nullptr),
               "Vulkan: Failed to Acquire Surface Present Modes!");
        std::vector<vk::PresentModeKHR> presentModes(presentModeCount);
        VkCall(physicalDevice.getSurfacePresentModesKHR(surface.GetSurface(), &presentModeCount, presentModes.data()),
               "Vulkan: Failed to Acquire Surface Present Modes!");

        presentMode = vk::PresentModeKHR::eFifo;
        for (const auto& presentMode : presentModes)
        {
            if (presentMode == vk::PresentModeKHR::eMailbox) this->presentMode = presentMode;
        }

        vk::SurfaceFormatKHR surfaceFormat = surface.GetSurfaceFormat();

        vk::SwapchainCreateInfoKHR swapChainCreateInfo = {};
        swapChainCreateInfo.sType = vk::StructureType::eSwapchainCreateInfoKHR;
        swapChainCreateInfo.pNext = VK_NULL_HANDLE;
        swapChainCreateInfo.flags = vk::SwapchainCreateFlagBitsKHR();
        swapChainCreateInfo.surface = surface.GetSurface();
        swapChainCreateInfo.minImageCount = surface.GetSurfaceCapabilities().minImageCount + 1;
        swapChainCreateInfo.imageFormat = surfaceFormat.format;
        swapChainCreateInfo.imageColorSpace = surfaceFormat.colorSpace;
        swapChainCreateInfo.imageExtent = this->extent;
        swapChainCreateInfo.imageArrayLayers = 1;
        swapChainCreateInfo.imageUsage = vk::ImageUsageFlagBits::eColorAttachment;

        uint32 queueFamilies[] = { device.GetGraphicsQueueFamily(), device.GetPresentQueueFamily() };
        if (device.GetGraphicsQueueFamily() != device.GetPresentQueueFamily())
        {
            swapChainCreateInfo.imageSharingMode = vk::SharingMode::eConcurrent;
            swapChainCreateInfo.queueFamilyIndexCount = 2;
            swapChainCreateInfo.pQueueFamilyIndices = queueFamilies;
        }
        else
        {
            swapChainCreateInfo.imageSharingMode = vk::SharingMode::eExclusive;
            swapChainCreateInfo.queueFamilyIndexCount = 0;
            swapChainCreateInfo.pQueueFamilyIndices = VK_NULL_HANDLE;
        }
        swapChainCreateInfo.preTransform = surface.GetSurfaceCapabilities().currentTransform;
        swapChainCreateInfo.compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque;
        swapChainCreateInfo.presentMode = presentMode;
        swapChainCreateInfo.clipped = VK_TRUE;
        swapChainCreateInfo.oldSwapchain = VK_NULL_HANDLE;

        VkCall(device.GetLogicalDevice().createSwapchainKHR(&swapChainCreateInfo, VkAllocator::Get().callbacks, &swapChain),
               "Vulkan: Failed to Create SwapChain!");
        VT_CORE_ASSERT_MSG(swapChain, "Vulkan: Failed to Create SwapChain!");

        uint32 imageCount = 0;
        VkCall(device.GetLogicalDevice().getSwapchainImagesKHR(swapChain, &imageCount, nullptr),
               "Vulkan: Failed to Acquire Swap Chain Images!");
        images.resize(imageCount);
        VkCall(device.GetLogicalDevice().getSwapchainImagesKHR(swapChain, &imageCount, images.data()),
                "Vulkan: Failed to Acquire Swap Chain Images!");

        imageFormat = surfaceFormat.format;
    }
    void VkSwapChain::CreateImageViews()
    {
        vk::Device device = VkRendererAPI::GetDevice().GetLogicalDevice();
        imageViews.resize(images.size());

        for (int i = 0; i < imageViews.size(); i++)
        {
            vk::ImageViewCreateInfo imageViewCreateInfo = {};
            imageViewCreateInfo.sType = vk::StructureType::eImageViewCreateInfo;
            imageViewCreateInfo.pNext = nullptr;
            imageViewCreateInfo.flags = vk::ImageViewCreateFlagBits();
            imageViewCreateInfo.image = images[i];
            imageViewCreateInfo.viewType = vk::ImageViewType::e2D;
            imageViewCreateInfo.format = imageFormat;
            imageViewCreateInfo.components.r = vk::ComponentSwizzle::eIdentity;
            imageViewCreateInfo.components.g = vk::ComponentSwizzle::eIdentity;
            imageViewCreateInfo.components.b = vk::ComponentSwizzle::eIdentity;
            imageViewCreateInfo.components.a = vk::ComponentSwizzle::eIdentity;
            imageViewCreateInfo.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
            imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
            imageViewCreateInfo.subresourceRange.levelCount = 1;
            imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
            imageViewCreateInfo.subresourceRange.layerCount = 1;

            VkCall(device.createImageView(&imageViewCreateInfo, VkAllocator::Get().callbacks, &imageViews[i]),
                   "Vulkan: Failed to Create SwapChain Image View!");
        }
    }
}
