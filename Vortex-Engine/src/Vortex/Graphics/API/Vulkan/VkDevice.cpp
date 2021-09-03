//
// Created by vitriol1744 on 30.08.2021.
//
#include "VkDevice.hpp"

#include "Vortex/Graphics/API/Vulkan/VkRendererAPI.hpp"

#include <set>

namespace Vortex::Graphics
{
    static std::vector<const char*> deviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };

    VkDevice::~VkDevice()
    {
        device.destroy(VkAllocator::Get().callbacks);
    }

    void VkDevice::Initialize(VkInstance& vkInstance, VkSurface& surface)
    {
        static vk::Bool32 initialized = VK_FALSE;
        if (initialized) return;

        PickPhysicalDevice(vkInstance, surface);
        vk::PhysicalDeviceProperties physicalDeviceProperties;
        physicalDevice.getProperties(&physicalDeviceProperties);
        vk::PhysicalDeviceMemoryProperties memoryProperties;
        physicalDevice.getMemoryProperties(&memoryProperties);

        VTCoreLogInfo("Graphics Card: {}", physicalDeviceProperties.deviceName);
        VTCoreLogInfo("VRAM: {}MB", physicalDeviceVRAM / 1024.0f / 1024.0f);

        VTCoreLogTrace("Vulkan: Creating Logical Device...");
        CreateLogicalDevice();
        VTCoreLogTrace("Vulkan: Logical Device Created!");
        initialized = VK_TRUE;
    }

    void VkDevice::PickPhysicalDevice(VkInstance& vkInstance, VkSurface& surface)
    {
        uint32 deviceCount = 0;
        VkCall(vkInstance.instance.enumeratePhysicalDevices(&deviceCount, nullptr), "Failed to Enumerate through Physical Devices!");
        VT_CORE_ASSERT_MSG(deviceCount, "Failed to find GPU's with Vulkan Support!");

        std::vector<vk::PhysicalDevice> physicalDevices(deviceCount);
        VkCall(vkInstance.instance.enumeratePhysicalDevices(&deviceCount, physicalDevices.data()), "Failed to Enumerate through Physical Devices!");

        vk::PhysicalDevice bestDevice = VK_NULL_HANDLE;
        uint32 topScore = 0;

        for (auto& device : physicalDevices)
        {
            physicalDevice = device;
            int score = 0;
            vk::PhysicalDeviceProperties properties;
            device.getProperties(&properties);

            if (properties.deviceType == vk::PhysicalDeviceType::eDiscreteGpu) score += 1000;
            vk::PhysicalDeviceMemoryProperties memoryProperties;
            device.getMemoryProperties(&memoryProperties);

            auto heapsPointer = memoryProperties.memoryHeaps;
            auto heaps = std::vector<vk::MemoryHeap>(heapsPointer.begin(), heapsPointer.begin() + memoryProperties.memoryHeapCount);
            uint64 vram = 0;

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
    void VkDevice::FindQueueFamilies(VkSurface& surface)
    {
        uint32 queueFamilyCount = 0;
        physicalDevice.getQueueFamilyProperties(&queueFamilyCount, VK_NULL_HANDLE);
        std::vector<vk::QueueFamilyProperties> queueFamilies(queueFamilyCount);
        physicalDevice.getQueueFamilyProperties(&queueFamilyCount, queueFamilies.data());

        for (uint32 i = 0; i < queueFamilies.size(); i++)
        {
            if (queueFamilies[i].queueFlags & vk::QueueFlagBits::eGraphics) graphicsQueueFamily = i;

            vk::Bool32 surfaceSupport;
            VkCall(physicalDevice.getSurfaceSupportKHR(i, surface.GetSurface(), &surfaceSupport), "Failed to query Surface Support!");

            if (queueFamilies[i].queueCount > 0 && surfaceSupport) presentQueueFamily = i;
            if (queueFamilies[i].queueFlags & vk::QueueFlagBits::eCompute) computeQueueFamily = i;
            if (queueFamilies[i].queueFlags & vk::QueueFlagBits::eTransfer) transferQueueFamily = i;
        }
    }
    void VkDevice::CreateLogicalDevice()
    {
        const std::vector<const char*>& validationLayers = VkRendererAPI::GetValidationLayers();
        const vk::Bool32 useValidationLayers = VkRendererAPI::UseValidationLayers();

        std::set<uint32> queueFamilies =
        {
            graphicsQueueFamily.value(), presentQueueFamily.value(),
            //computeQueueFamily.value(), transferQueueFamily.value()
        };
        std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos;

        float32 queuePriority = 1.0f;
        for (uint32 queueFamily : queueFamilies)
        {
            vk::DeviceQueueCreateInfo queueCreateInfo = {};
            queueCreateInfo.sType = vk::StructureType::eDeviceQueueCreateInfo;
            queueCreateInfo.pNext = VK_NULL_HANDLE;
            queueCreateInfo.flags = vk::DeviceQueueCreateFlags();
            queueCreateInfo.queueFamilyIndex = queueFamily;
            queueCreateInfo.queueCount = 1;
            queueCreateInfo.pQueuePriorities = &queuePriority;
            queueCreateInfos.push_back(queueCreateInfo);
        }

        vk::DeviceCreateInfo deviceCreateInfo = {};
        deviceCreateInfo.sType = vk::StructureType::eDeviceCreateInfo;
        deviceCreateInfo.pNext = nullptr;
        deviceCreateInfo.flags = vk::DeviceCreateFlagBits();
        deviceCreateInfo.queueCreateInfoCount = queueCreateInfos.size();
        deviceCreateInfo.pQueueCreateInfos = queueCreateInfos.data();
        deviceCreateInfo.enabledLayerCount = useValidationLayers ? validationLayers.size() : 0;
        deviceCreateInfo.ppEnabledLayerNames = useValidationLayers ? validationLayers.data() : nullptr;
        deviceCreateInfo.enabledExtensionCount = deviceExtensions.size();
        deviceCreateInfo.ppEnabledExtensionNames = deviceExtensions.data();
        deviceCreateInfo.pEnabledFeatures = &physicalDeviceFeatures;

        VkCall(physicalDevice.createDevice(&deviceCreateInfo, VkAllocator::Get().callbacks, &device),
               "Failed to Create Logical Device!");

        device.getQueue(graphicsQueueFamily.value(), 0, &graphicsQueue);
        device.getQueue(presentQueueFamily.value(), 0, &presentQueue);
       //device.getQueue(computeQueueFamily.value(), 0, &computeQueue);
       //device.getQueue(transferQueueFamily.value(), 0, &transferQueue);
    }

    vk::Bool32 VkDevice::IsDeviceSuitable(vk::PhysicalDevice physicalDevice)
    {
        vk::PhysicalDeviceFeatures deviceFeatures;
        physicalDevice.getFeatures(&deviceFeatures);

        return deviceFeatures.geometryShader && CheckDeviceExtensionsSupport(physicalDevice);
    }
    vk::Bool32 VkDevice::CheckDeviceExtensionsSupport(vk::PhysicalDevice physicalDevice)
    {
        uint32 extensionCount = 0;
        physicalDevice.enumerateDeviceExtensionProperties(nullptr, &extensionCount, nullptr);
        std::vector<vk::ExtensionProperties> availableExtensions(extensionCount);
        physicalDevice.enumerateDeviceExtensionProperties(nullptr, &extensionCount, availableExtensions.data());

        std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

        for (const auto& extension : availableExtensions)
        {
            requiredExtensions.erase(extension.extensionName);
        }

        return requiredExtensions.empty();
    }
}