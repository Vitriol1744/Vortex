//
// Created by Vitriol1744 on 14.09.2022.
//
#ifndef VORTEX_VULKANSURFACE_HPP
#define VORTEX_VULKANSURFACE_HPP

#include "Vortex/Graphics/API/Vulkan/VulkanCommon.hpp"
#include "Vortex/Graphics/API/Vulkan/VulkanRendererAPI.hpp"

//FIXME: Temporary
#include <GLFW/glfw3.h>

namespace Vortex::Graphics
{
    class VT_API VulkanSurface
    {
        public:
            void Create(GLFWwindow* windowHandle);
            void Destroy();

            inline VkSurfaceKHR GetSurface() const noexcept { return surface; }
            inline VkSurfaceFormatKHR GetFormat() const noexcept { return format; }
            VkSurfaceCapabilitiesKHR GetCapabilities()
            {
                VkPhysicalDevice physicalDevice = VulkanRendererAPI::GetDevice().GetPhysicalDevice();
                VkSurfaceCapabilitiesKHR capabilities;
                VkCall_msg(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &capabilities),
                                                                                        "Failed to Acquire Surface Capabilities!");
                return capabilities;
            }

        private:
            VkSurfaceKHR surface = VK_NULL_HANDLE;
            VkSurfaceFormatKHR format;
    };
}
#endif //VORTEX_VULKANSURFACE_HPP
