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
        device.destroySwapchainKHR(swapChain, VkAllocator::Get().callbacks);
    }

    void VkSwapChain::Initialize(const vk::Extent2D extent, const VkSurface& surface)
    {
        this->extent = extent;
        VkDevice& device = VkRendererAPI::GetDevice();
        vk::PhysicalDevice& physicalDevice = device.GetPhysicalDevice();

        uint32 presentModeCount = 0;
        VkCall(physicalDevice.getSurfacePresentModesKHR(surface.GetSurface(), &presentModeCount, nullptr),
               "Failed to Acquire Surface Present Modes!");
        std::vector<vk::PresentModeKHR> presentModes(presentModeCount);
        VkCall(physicalDevice.getSurfacePresentModesKHR(surface.GetSurface(), &presentModeCount, presentModes.data()),
               "Failed to Acquire Surface Present Modes!");

        presentMode = vk::PresentModeKHR::eFifo;
        for (const auto& presentMode : presentModes)
        {
            if (presentMode == vk::PresentModeKHR::eMailbox) this->presentMode = presentMode;
        }

        vk::SwapchainCreateInfoKHR swapChainCreateInfo = {};
        swapChainCreateInfo.sType = vk::StructureType::eSwapchainCreateInfoKHR;
        swapChainCreateInfo.pNext = VK_NULL_HANDLE;
        swapChainCreateInfo.flags = vk::SwapchainCreateFlagBitsKHR();
        swapChainCreateInfo.surface = surface.GetSurface();
        swapChainCreateInfo.minImageCount = surface.GetSurfaceCapabilities().minImageCount + 1;
        swapChainCreateInfo.imageFormat = surface.GetSurfaceFormat().format;
        swapChainCreateInfo.imageColorSpace = surface.GetSurfaceFormat().colorSpace;
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
               "Failed to Create SwapChain!");
        //VT_CORE_ASSERT_MSG(swapChain, "Failed to Create SwapChain!");

        //uint32 imageCount = 0;
        //device.GetLogicalDevice().getSwapchainImagesKHR(swapChain, &imageCount, nullptr);
        //images.resize(imageCount);
        //device.GetLogicalDevice().getSwapchainImagesKHR(swapChain, &imageCount, images.data());
    }
}
