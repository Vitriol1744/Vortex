/*
 * Created by v1tr10l7 on 19.07.2024.
 * Copyright (c) 2024-2024, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#include "vtpch.hpp"

#include "Vortex/Renderer/API/Vulkan/VulkanContext.hpp"
#include "Vortex/Renderer/API/Vulkan/VulkanDevice.hpp"

#include <GLFW/glfw3.h>

namespace Vortex
{
    static std::array<const char*, 1> s_DeviceExtensions
        = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};

    VulkanPhysicalDevice VulkanPhysicalDevice::Pick()
    {
        VtCoreTrace("Vulkan: Picking physical device");
        VulkanPhysicalDevice ret;
        vk::Instance vkInstance = vk::Instance(VulkanContext::GetInstance());

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
            ret.m_PhysicalDevice               = device;
            u32                          score = 0;
            vk::PhysicalDeviceProperties properties;
            ret.m_PhysicalDevice.getProperties(&properties);

            if (properties.deviceType == vk::PhysicalDeviceType::eDiscreteGpu)
                score += 1000;
            vk::PhysicalDeviceMemoryProperties memoryProperties{};
            ret.m_PhysicalDevice.getMemoryProperties(&memoryProperties);

            auto  heaps = memoryProperties.memoryHeaps;
            usize vram  = 0;

            for (auto& heap : heaps)
            {
                if (heap.flags & vk::MemoryHeapFlagBits::eDeviceLocal)
                {
                    vram = heap.size;
                    score += static_cast<u32>(vram);
                    break;
                }
            }

            ret.FindQueueFamilies();
            if (ret.IsDeviceSuitable(device) && score > topScore
                && ret.m_QueueFamilyIndices.Graphics.has_value()
                && ret.m_QueueFamilyIndices.Present.has_value())
            {
                bestDevice               = device;
                topScore                 = score;
                ret.m_PhysicalDeviceVRAM = vram;
            }
        }

        ret.m_PhysicalDevice = bestDevice;
        VtCoreAssertMsg(bestDevice, "Failed to find suitable physical device");
        VtCoreTrace("Vulkan: Picked physical device, available memory: {}MB",
                    ret.m_PhysicalDeviceVRAM / 1024 / 1024);
        return ret;
    }

    u32 VulkanPhysicalDevice::FindMemoryType(
        u32 typeFilter, vk::MemoryPropertyFlags properties) const
    {
        vk::PhysicalDeviceMemoryProperties memoryProperties{};
        m_PhysicalDevice.getMemoryProperties(&memoryProperties);

        for (u32 i = 0; i < memoryProperties.memoryTypeCount; i++)
        {
            if (typeFilter & (1 << i)
                && (memoryProperties.memoryTypes[i].propertyFlags & properties)
                       == properties)
                return i;
        }

        VtCoreError("Failed to find suitable memory type!");
        VtCoreAssert(false);
        return 0;
    }

    void VulkanPhysicalDevice::FindQueueFamilies()
    {
        u32 queueFamilyCount = 0;
        m_PhysicalDevice.getQueueFamilyProperties(&queueFamilyCount,
                                                  VK_NULL_HANDLE);
        std::vector<vk::QueueFamilyProperties> queueFamilies(queueFamilyCount);
        m_PhysicalDevice.getQueueFamilyProperties(&queueFamilyCount,
                                                  queueFamilies.data());

        VkInstance instance = vk::Instance(VulkanContext::GetInstance());
        for (u32 i = 0; i < queueFamilies.size(); i++)
        {
            if (queueFamilies[i].queueFlags & vk::QueueFlagBits::eGraphics)
                m_QueueFamilyIndices.Graphics = i;

            if (queueFamilies[i].queueCount > 0
                && glfwGetPhysicalDevicePresentationSupport(
                    instance, m_PhysicalDevice, i))
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

    void VulkanDevice::Initialize(const VulkanPhysicalDevice& physicalDevice)
    {
        VtCoreTrace("Vulkan: Creating logical device...");
        m_PhysicalDevice = physicalDevice;

        const std::vector<const char*>& validationLayers
            = VulkanInstance::GetValidationLayers();
        const vk::Bool32 useValidationLayers
            = VulkanInstance::ShouldUseValidationLayers();

        VulkanPhysicalDevice::QueueFamilyIndices queueFamilyIndices
            = physicalDevice.GetQueueFamilyIndices();
        std::set<u32> queueFamilies;
        queueFamilies.insert(queueFamilyIndices.Graphics.value());
        queueFamilies.insert(queueFamilyIndices.Present.value());
        if (queueFamilyIndices.Compute.has_value())
            queueFamilies.insert(queueFamilyIndices.Compute.value());
        if (queueFamilyIndices.Transfer.has_value())
            queueFamilies.insert(queueFamilyIndices.Transfer.value());

        std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos;

        f32                                    priority = 1.0f;
        for (u32 queueFamily : queueFamilies)
        {
            vk::DeviceQueueCreateInfo queueCreateInfo = {};
            queueCreateInfo.sType = vk::StructureType::eDeviceQueueCreateInfo;
            queueCreateInfo.pNext = VK_NULL_HANDLE;
            queueCreateInfo.flags = vk::DeviceQueueCreateFlags();
            queueCreateInfo.queueFamilyIndex = queueFamily;
            queueCreateInfo.queueCount       = 1;
            queueCreateInfo.pQueuePriorities = &priority;
            queueCreateInfos.push_back(queueCreateInfo);
        }

        vk::DeviceCreateInfo       deviceCreateInfo = {};
        vk::PhysicalDeviceFeatures features;
        deviceCreateInfo.sType = vk::StructureType::eDeviceCreateInfo;
        deviceCreateInfo.pNext = nullptr;
        deviceCreateInfo.flags = vk::DeviceCreateFlagBits();
        deviceCreateInfo.queueCreateInfoCount
            = static_cast<u32>(queueCreateInfos.size());
        deviceCreateInfo.pQueueCreateInfos = queueCreateInfos.data();
        deviceCreateInfo.enabledLayerCount
            = useValidationLayers ? static_cast<u32>(validationLayers.size())
                                  : 0ull;
        deviceCreateInfo.ppEnabledLayerNames
            = useValidationLayers ? validationLayers.data() : nullptr;
        deviceCreateInfo.enabledExtensionCount
            = static_cast<u32>(s_DeviceExtensions.size());
        deviceCreateInfo.ppEnabledExtensionNames = s_DeviceExtensions.data();
        deviceCreateInfo.pEnabledFeatures        = &features;

        VkCall(((vk::PhysicalDevice)(physicalDevice))
                   .createDevice(&deviceCreateInfo, nullptr, &m_Device));

        m_Device.getQueue(queueFamilyIndices.Graphics.value(), 0,
                          &m_GraphicsQueue);
        m_Device.getQueue(queueFamilyIndices.Present.value(), 0,
                          &m_PresentQueue);
        if (queueFamilyIndices.Compute.has_value())
            m_Device.getQueue(queueFamilyIndices.Compute.value(), 0,
                              &m_ComputeQueue);
        if (queueFamilyIndices.Transfer.has_value())
            m_Device.getQueue(queueFamilyIndices.Transfer.value(), 0,
                              &m_TransferQueue);
    }
    void VulkanDevice::Destroy()
    {
        VtCoreTrace("Vulkan: Destroying logical device...");
        m_Device.destroy(nullptr);
    }
}; // namespace Vortex
