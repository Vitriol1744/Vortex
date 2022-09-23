//
// Created by Vitriol1744 on 14.09.2022.
//
#ifndef VORTEX_VULKANSWAPCHAIN_HPP
#define VORTEX_VULKANSWAPCHAIN_HPP

#include "Vortex/Graphics/API/Vulkan/VulkanCommon.hpp"

#include "Vortex/Graphics/API/Vulkan/VulkanRendererAPI.hpp"
#include "Vortex/Graphics/API/Vulkan/VulkanSurface.hpp"

namespace Vortex::Graphics
{
    class VT_API VulkanSwapChain
    {
        public:
            inline void Create(const VulkanSurface& surface, uint32 width, uint32 height)
            {
                CreateSwapChain(surface, width, height);
                CreateImageViews();
            }
            inline void Destroy()
            {
                VkDevice device = VulkanRendererAPI::GetDevice().GetLogicalDevice();
                for (auto imageView : imageViews) vkDestroyImageView(device, imageView, nullptr);
                imageViews.clear();
                vkDestroySwapchainKHR(device, swapChain, nullptr);
                images.clear();
            }

            void CreateSwapChain(VulkanSurface surface, uint32 width, uint32 height);
            void CreateImageViews();

            inline VkSwapchainKHR GetSwapChain() const noexcept { return swapChain; }
            inline VkPresentModeKHR GetPresentMode() const noexcept { return presentMode; }
            inline VkExtent2D& GetExtent() noexcept { return extent; }
            inline VkFormat GetImageFormat() const noexcept { return imageFormat; }
            inline uint32& GetImageIndex() noexcept { return imageIndex; }

            inline std::vector<VkImage>& GetImages() noexcept { return images; }
            inline std::vector<VkImageView>& GetImageViews() noexcept { return imageViews; }

        private:
            VkSwapchainKHR swapChain;
            VkPresentModeKHR presentMode;
            VkExtent2D extent;
            VkFormat imageFormat;
            uint32 imageIndex;

            std::vector<VkImage> images;
            std::vector<VkImageView> imageViews;
    };
}

#endif //VORTEX_VULKANSWAPCHAIN_HPP
