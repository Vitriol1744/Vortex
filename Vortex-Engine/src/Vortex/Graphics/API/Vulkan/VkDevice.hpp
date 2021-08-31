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

        private:
            vk::PhysicalDevice physicalDevice = VK_NULL_HANDLE;
            vk::PhysicalDeviceFeatures physicalDeviceFeatures;

            vk::Device device = VK_NULL_HANDLE;

            std::optional<uint32> graphicsQueueFamily;
            std::optional<uint32> presentQueueFamily;
            std::optional<uint32> computeQueueFamily;
            std::optional<uint32> transferQueueFamily;

            vk::Queue graphicsQueue;

            void PickPhysicalDevice(VkInstance& vkInstance, VkSurface& surface);
            void FindQueueFamilies(VkSurface& surface);
            void CreateLogicalDevice();

            vk::Bool32 IsDeviceSuitable(vk::PhysicalDevice physicalDevice);
    };
}
