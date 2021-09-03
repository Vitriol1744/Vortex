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

            inline const vk::SurfaceKHR& GetSurface() const noexcept { return surface; }
            inline const vk::SurfaceFormatKHR& GetSurfaceFormat() const noexcept { return surfaceFormat; }
            inline const vk::SurfaceCapabilitiesKHR& GetSurfaceCapabilities() const noexcept { return surfaceCapabilities; }

        private:
            vk::SurfaceKHR surface = VK_NULL_HANDLE;
            vk::SurfaceFormatKHR surfaceFormat;
            vk::SurfaceCapabilitiesKHR surfaceCapabilities;
    };
}