/*
 * Created by v1tr10l7 on 19.07.2024.
 * Copyright (c) 2024-2024, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#include "vtpch.hpp"

#include "Vortex/Graphics/API/Vulkan/VulkanDevice.hpp"

namespace Vortex
{
    static std::array<const char*, 1> s_DeviceExtensions
        = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};

    VulkanPhysicalDevice::VulkanPhysicalDevice(const VulkanInstance& instance,
                                               const VulkanSurface&  surface)
    {
        vk::Instance vkInstance = (vk::Instance)instance;

        u32          gpuCount   = 0;
        VkCall(vkInstance.enumeratePhysicalDevices(&gpuCount, VK_NULL_HANDLE));
        VtCoreAssert(gpuCount > 0);
        std::vector<vk::PhysicalDevice> physicalDevices(gpuCount);
        VkCall(vkInstance.enumeratePhysicalDevices(&gpuCount,
                                                   physicalDevices.data()));

        vk::PhysicalDevice bestDevice = VK_NULL_HANDLE;
        usize              topScore   = 0;
        for (auto device : physicalDevices)
        {
            m_PhysicalDevice                   = device;
            u32                          score = 0;
            vk::PhysicalDeviceProperties properties;
            m_PhysicalDevice.getProperties(&properties);

            if (properties.deviceType == vk::PhysicalDeviceType::eDiscreteGpu)
                score += 1000;
            vk::PhysicalDeviceMemoryProperties memoryProperties{};
            m_PhysicalDevice.getMemoryProperties(&memoryProperties);

            auto  heaps = memoryProperties.memoryHeaps;
            usize vram  = 0;

            for (auto& heap : heaps)
            {
                if (heap.flags & vk::MemoryHeapFlagBits::eDeviceLocal)
                {
                    vram = heap.size;
                    score += vram;
                    break;
                }
            }

            FindQueueFamilies(surface);
            if (IsDeviceSuitable(device) && score > topScore
                && m_QueueFamilyIndices.Graphics.has_value()
                && m_QueueFamilyIndices.Present.has_value())
            {
                bestDevice           = device;
                topScore             = score;
                m_PhysicalDeviceVRAM = vram;
            }
        }

        m_PhysicalDevice = bestDevice;
        VtCoreAssertMsg(bestDevice, "Failed to find suitable physical device");
        VtCoreTrace("Vulkan: Picked physical device");
    }

    void VulkanPhysicalDevice::FindQueueFamilies(const VulkanSurface& surface)
    {
        u32 queueFamilyCount = 0;
        m_PhysicalDevice.getQueueFamilyProperties(&queueFamilyCount,
                                                  VK_NULL_HANDLE);
        std::vector<vk::QueueFamilyProperties> queueFamilies(queueFamilyCount);
        m_PhysicalDevice.getQueueFamilyProperties(&queueFamilyCount,
                                                  queueFamilies.data());

        for (u32 i = 0; i < queueFamilies.size(); i++)
        {
            if (queueFamilies[i].queueFlags & vk::QueueFlagBits::eGraphics)
                m_QueueFamilyIndices.Graphics = i;

            vk::Bool32 surfaceSupport;
            VkCall(m_PhysicalDevice.getSurfaceSupportKHR(i, surface,
                                                         &surfaceSupport));

            if (queueFamilies[i].queueCount > 0 && surfaceSupport)
                m_QueueFamilyIndices.Present = i;
            if (queueFamilies[i].queueFlags & vk::QueueFlagBits::eCompute)
                m_QueueFamilyIndices.Compute = i;
            if (queueFamilies[i].queueFlags & vk::QueueFlagBits::eTransfer)
                m_QueueFamilyIndices.Transfer = i;
        }
    }

    vk::Bool32
    VulkanPhysicalDevice::IsDeviceSuitable(vk::PhysicalDevice physDevice)
    {
        vk::PhysicalDeviceFeatures features;
        physDevice.getFeatures(&features);

        return features.geometryShader
            && CheckDeviceExtensionSupport(physDevice);
    }

    vk::Bool32 VulkanPhysicalDevice::CheckDeviceExtensionSupport(
        vk::PhysicalDevice physDevice)
    {
        u32 extensionCount = 0;
        VkCall(physDevice.enumerateDeviceExtensionProperties(
            nullptr, &extensionCount, nullptr));
        std::vector<vk::ExtensionProperties> availableExtensions(
            extensionCount);
        VkCall(physDevice.enumerateDeviceExtensionProperties(
            nullptr, &extensionCount, availableExtensions.data()));

        std::set<std::string> requiredExtensions(s_DeviceExtensions.begin(),
                                                 s_DeviceExtensions.end());

        for (const auto& extension : availableExtensions)
            requiredExtensions.erase(extension.extensionName);

        return requiredExtensions.empty();
    }
}; // namespace Vortex
