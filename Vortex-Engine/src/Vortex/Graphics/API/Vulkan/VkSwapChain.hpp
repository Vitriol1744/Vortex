//
// Created by vitriol1744 on 02.09.2021.
//
#pragma once

#include "Vortex/Core/Core.hpp"

#include "Vortex/Graphics/API/Vulkan/VkCommon.hpp"
#include "Vortex/Graphics/API/Vulkan/VkSurface.hpp"

namespace Vortex::Graphics
{
    class VT_API VkSwapChain
    {
        public:
            VkSwapChain() = default;
            ~VkSwapChain();

            void Initialize(const vk::Extent2D extent, const VkSurface& surface);

        private:
            vk::SwapchainKHR swapChain;
            vk::PresentModeKHR presentMode;
            vk::Extent2D extent;

            std::vector<vk::Image> images;
            std::vector<vk::ImageView> imageViews;
    };
}