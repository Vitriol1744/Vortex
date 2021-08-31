//
// Created by vitriol1744 on 31.08.2021.
//
#pragma once

#include "Vortex/Core/Core.hpp"

#include "Vortex/Graphics/API/Vulkan/VkCommon.hpp"

namespace Vortex::Graphics
{
    class VT_API VkSurface
    {
        public:
            VkSurface() = default;
            ~VkSurface();

            void Initialize(Platform::NativeWindowHandleType windowHandle);

            vk::SurfaceKHR GetSurface() const noexcept { return surface; }
            vk::SurfaceFormatKHR GetSurfaceFormat() const noexcept { return surfaceFormat; }

        private:
            vk::SurfaceKHR surface = VK_NULL_HANDLE;
            vk::SurfaceFormatKHR surfaceFormat;
    };
}