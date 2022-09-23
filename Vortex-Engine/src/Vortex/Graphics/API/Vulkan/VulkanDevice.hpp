//
// Created by Vitriol1744 on 14.09.2022.
//
#ifndef VORTEX_VULKANDEVICE_HPP
#define VORTEX_VULKANDEVICE_HPP

#include "Vortex/Graphics/API/Vulkan/VulkanCommon.hpp"

#include "Vortex/Graphics/API/Vulkan/VulkanInstance.hpp"

namespace Vortex::Graphics
{
    class VT_API VulkanDevice
    {
        public:
            void Create(VkSurfaceKHR surface);
            inline void Destroy() { vkDestroyDevice(device, VulkanAllocator::Get().callbacks); }

            inline VkPhysicalDevice& GetPhysicalDevice() noexcept { return physicalDevice; }
            inline VkDevice& GetLogicalDevice() noexcept { return device; }

            inline uint32 GetGraphicsQueueFamily() const noexcept { return graphicsQueueFamily.value(); }
            inline uint32 GetPresentQueueFamily() const noexcept { return presentQueueFamily.value(); }
            inline uint32 GetComputeQueueFamily() const noexcept { return computeQueueFamily.value(); }
            inline uint32 GetTransferQueueFamily() const noexcept { return transferQueueFamily.value(); }

            inline VkQueue GetGraphicsQueue() const noexcept { return graphicsQueue; }
            inline VkQueue GetPresentQueue() const noexcept { return presentQueue; }
            inline VkQueue GetTransferQueue() const noexcept { return graphicsQueue; }

        private:
            VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
            VkPhysicalDeviceFeatures physicalDeviceFeatures = {};
            uint64 physicalDeviceVRAM = 0;

            VkDevice device;

            std::optional<uint32> graphicsQueueFamily;
            std::optional<uint32> presentQueueFamily;
            std::optional<uint32> computeQueueFamily;
            std::optional<uint32> transferQueueFamily;

            VkQueue graphicsQueue = VK_NULL_HANDLE;
            VkQueue presentQueue = VK_NULL_HANDLE;
            VkQueue computeQueue = VK_NULL_HANDLE;
            VkQueue transferQueue = VK_NULL_HANDLE;

            void PickPhysicalDevice(VkSurfaceKHR& surface);
            void FindQueueFamilies(VkSurfaceKHR& surface);
            void CreateLogicalDevice();

            VkBool32 IsDeviceSuitable(VkPhysicalDevice physicalDevice);
            VkBool32 CheckDeviceExtensionsSupport(VkPhysicalDevice physicalDevice);
    };
}

#endif //VORTEX_VULKANDEVICE_HPP
