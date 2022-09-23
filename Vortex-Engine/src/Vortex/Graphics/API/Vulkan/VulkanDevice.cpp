//
// Created by Vitriol1744 on 14.09.2022.
//
#include "VulkanDevice.hpp"

#include "Vortex/Graphics/API/Vulkan/VulkanRendererAPI.hpp"

namespace Vortex::Graphics
{
    static std::vector<const char*> deviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };

    void VulkanDevice::Create(VkSurfaceKHR surface)
    {
        VkInstance& vulkanInstance = VulkanRendererAPI::GetVulkanInstance().instance;

        PickPhysicalDevice(surface);
        VkPhysicalDeviceProperties physicalDeviceProperties;
        vkGetPhysicalDeviceProperties(physicalDevice, &physicalDeviceProperties);
        VkPhysicalDeviceMemoryProperties memoryProperties;
        vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memoryProperties);

        VTCoreLogInfo("Graphics Card: {}", physicalDeviceProperties.deviceName);
        VTCoreLogInfo("VRAM: {}MB", physicalDeviceVRAM / 1024.0f / 1024.0f);

        VTCoreLogTrace("Vulkan: Creating Logical Device...");
        CreateLogicalDevice();
        VTCoreLogTrace("Vulkan: Logical Device Created!");
    }

    void VulkanDevice::PickPhysicalDevice(VkSurfaceKHR& surface)
    {
        VkInstance& vulkanInstance = VulkanRendererAPI::GetVulkanInstance().instance;
        uint32 deviceCount = 0;
        VkCall_msg(vkEnumeratePhysicalDevices(vulkanInstance, &deviceCount, nullptr),
                   "Failed to Enumerate through Physical Devices!");
        VT_CORE_ASSERT_MSG(deviceCount, "Failed to find GPU\'s with Vulkan Support!");

        std::vector<VkPhysicalDevice> physicalDevices(deviceCount);
        VkCall_msg(vkEnumeratePhysicalDevices(vulkanInstance, &deviceCount, physicalDevices.data()),
                   "Failed to Enumerate through Physical Devices!");

        VkPhysicalDevice bestDevice = VK_NULL_HANDLE;
        uint32 topScore = 0;

        for (auto& device : physicalDevices)
        {
            physicalDevice = device;
            uint32 score = 0;
            VkPhysicalDeviceProperties properties;
            vkGetPhysicalDeviceProperties(device, &properties);

            if (properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) score += 1000;
            VkPhysicalDeviceMemoryProperties memoryProperties;
            vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memoryProperties);

            auto heapsPointer = memoryProperties.memoryHeaps;
            auto heaps = std::vector<VkMemoryHeap>(heapsPointer, heapsPointer + memoryProperties.memoryHeapCount);
            uint64 vram = 0;

            for (auto& heap : heaps)
            {
                if (heap.flags & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT)
                {
                    vram = heap.size;
                    score += static_cast<uint32>(vram);
                    break;
                }
            }

            FindQueueFamilies(surface);
            if (IsDeviceSuitable(device) && score > topScore && graphicsQueueFamily.has_value() && presentQueueFamily.has_value())
            {
                bestDevice = device;
                topScore = score;
                physicalDeviceVRAM = vram;
            }
        }
        physicalDevice = bestDevice;
        VT_CORE_ASSERT_MSG(bestDevice, "Failed to find Suitable Device!");
        VTCoreLogTrace("Vulkan: Picked Physical Device!");
    }
    void VulkanDevice::FindQueueFamilies(VkSurfaceKHR& surface)
    {
        uint32 queueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, nullptr);
        std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueFamilies.data());

        for (uint32 i = 0; i < queueFamilies.size(); i++)
        {
            if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) graphicsQueueFamily = i;

            VkBool32 surfaceSupport;
            VkCall_msg(vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, i, surface, &surfaceSupport),
                       "Failed to query Surface Support!");

            if (queueFamilies[i].queueCount > 0 && surfaceSupport && !presentQueueFamily.has_value()) presentQueueFamily = i;
            if (queueFamilies[i].queueFlags & VK_QUEUE_COMPUTE_BIT && !computeQueueFamily.has_value()) computeQueueFamily = i;
            if (queueFamilies[i].queueFlags & VK_QUEUE_TRANSFER_BIT && !transferQueueFamily.has_value()) transferQueueFamily = i;
            if (graphicsQueueFamily.has_value() && presentQueueFamily.has_value() && computeQueueFamily.has_value() && transferQueueFamily.has_value())
                break;
        }
    }
    void VulkanDevice::CreateLogicalDevice()
    {
        const std::vector<const char*>& validationLayers = VulkanRendererAPI::GetValidationLayers();
        const VkBool32 useValidationLayers = VulkanRendererAPI::UseValidationLayers();

        std::set<uint32> queueFamilies =
        {
            graphicsQueueFamily.value(), presentQueueFamily.value(),
            computeQueueFamily.value(), transferQueueFamily.value()
        };
        std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;

        float32 queuePriority = 1.0f;
        for (uint32 queueFamily : queueFamilies)
        {
            VkDeviceQueueCreateInfo queueCreateInfo = {};
            queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queueCreateInfo.pNext = VK_NULL_HANDLE;
            queueCreateInfo.flags = 0;
            queueCreateInfo.queueFamilyIndex = queueFamily;
            queueCreateInfo.queueCount = 1;
            queueCreateInfo.pQueuePriorities = &queuePriority;
            queueCreateInfos.push_back(queueCreateInfo);
        }

        VkDeviceCreateInfo deviceCreateInfo = {};
        deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        deviceCreateInfo.pNext = nullptr;
        deviceCreateInfo.flags = 0;
        deviceCreateInfo.queueCreateInfoCount = queueCreateInfos.size();
        deviceCreateInfo.pQueueCreateInfos = queueCreateInfos.data();
        deviceCreateInfo.enabledLayerCount = useValidationLayers ? validationLayers.size() : 0;
        deviceCreateInfo.ppEnabledLayerNames = useValidationLayers ? validationLayers.data() : nullptr;
        deviceCreateInfo.enabledExtensionCount = deviceExtensions.size();
        deviceCreateInfo.ppEnabledExtensionNames = deviceExtensions.data();
        deviceCreateInfo.pEnabledFeatures = &physicalDeviceFeatures;

        VkCall_msg(vkCreateDevice(physicalDevice, &deviceCreateInfo, VulkanAllocator::Get().callbacks, &device),
                   "Failed to Create Logical Device!");

        vkGetDeviceQueue(device, graphicsQueueFamily.value(), 0, &graphicsQueue);
        vkGetDeviceQueue(device, presentQueueFamily.value(), 0, &presentQueue);
        vkGetDeviceQueue(device, computeQueueFamily.value(), 0, &computeQueue);
        vkGetDeviceQueue(device, transferQueueFamily.value(), 0, &transferQueue);
    }

    VkBool32 VulkanDevice::IsDeviceSuitable(VkPhysicalDevice _physicalDevice)
    {
        VkPhysicalDeviceFeatures deviceFeatures;
        vkGetPhysicalDeviceFeatures(_physicalDevice, &deviceFeatures);

        return deviceFeatures.geometryShader && CheckDeviceExtensionsSupport(physicalDevice);
    }
    VkBool32 VulkanDevice::CheckDeviceExtensionsSupport(VkPhysicalDevice physicalDevice)
    {
        uint32 extensionCount = 0;
        VkCall_msg(vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount, nullptr),
                   "Failed to Enumerate through Device Extensions!");
        std::vector<VkExtensionProperties> availableExtensions(extensionCount);
        VkCall_msg(vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount, availableExtensions.data()),
                   "Failed to Enumerate through Device Extensions!");

        std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

        for (const auto& extension : availableExtensions)
        {
            requiredExtensions.erase(extension.extensionName);
        }

        return requiredExtensions.empty();
    }
}
