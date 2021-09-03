//
// Created by vitriol1744 on 30.08.2021.
//
#pragma once

#include "Vortex/Core/Core.hpp"

#include "Vortex/Graphics/API/Vulkan/VkCommon.hpp"
#include "Vortex/Graphics/API/Vulkan/VkInstance.hpp"
#include "Vortex/Graphics/API/Vulkan/VkSurface.hpp"

#include <optional>

namespace Vortex::Graphics
{
    class VT_API VkDevice
    {
        public:
            VkDevice() = default;
            ~VkDevice();

            void Initialize(VkInstance& vkInstance, VkSurface& surface);

            inline vk::PhysicalDevice& GetPhysicalDevice() noexcept { return physicalDevice; }
            inline vk::Device& GetLogicalDevice() noexcept { return device; }

            inline uint32 GetGraphicsQueueFamily() noexcept { return graphicsQueueFamily.value(); }
            inline uint32 GetPresentQueueFamily() noexcept { return presentQueueFamily.value(); }
            inline uint32 GetComputeQueueFamily() noexcept { return computeQueueFamily.value(); }
            inline uint32 GetTransferQueueFamily() noexcept { return transferQueueFamily.value(); }

        private:
            vk::PhysicalDevice physicalDevice = VK_NULL_HANDLE;
            vk::PhysicalDeviceFeatures physicalDeviceFeatures;
            uint64 physicalDeviceVRAM = 0;

            vk::Device device = VK_NULL_HANDLE;

            std::optional<uint32> graphicsQueueFamily;
            std::optional<uint32> presentQueueFamily;
            std::optional<uint32> computeQueueFamily;
            std::optional<uint32> transferQueueFamily;

            vk::Queue graphicsQueue = VK_NULL_HANDLE;
            vk::Queue presentQueue = VK_NULL_HANDLE;
            vk::Queue computeQueue = VK_NULL_HANDLE;
            vk::Queue transferQueue = VK_NULL_HANDLE;

            void PickPhysicalDevice(VkInstance& vkInstance, VkSurface& surface);
            void FindQueueFamilies(VkSurface& surface);
            void CreateLogicalDevice();

            vk::Bool32 IsDeviceSuitable(vk::PhysicalDevice physicalDevice);
            vk::Bool32 CheckDeviceExtensionsSupport(vk::PhysicalDevice physicalDevice);
    };
}
