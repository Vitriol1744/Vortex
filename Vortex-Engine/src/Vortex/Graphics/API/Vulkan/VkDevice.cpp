//
// Created by vitriol1744 on 30.08.2021.
//
#include "VkDevice.hpp"

#include "Vortex/Graphics/API/Vulkan/VkRendererAPI.hpp"

#pragma clang diagnostic push
#pragma ide diagnostic ignored "ConstantConditionsOC"
namespace Vortex::Graphics
{
    VkDevice::~VkDevice()
    {
        device.destroy(VkAllocator::Get().callbacks);
    }

    void VkDevice::Initialize(VkInstance& vkInstance, VkSurface& surface)
    {
        static vk::Bool32 initialized = VK_FALSE;
        if (initialized) return;

        PickPhysicalDevice(vkInstance, surface);
        CreateLogicalDevice();
        
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

            for (auto& heap : heaps)
            {
                if (heap.flags & vk::MemoryHeapFlagBits::eDeviceLocal)
                {
                    score += heap.size; // VRAM
                    break;
                }
            }

            FindQueueFamilies(surface);
            if (IsDeviceSuitable(device) && score > topScore && graphicsQueueFamily.has_value() && presentQueueFamily.has_value())
            {
                bestDevice = device;
                topScore = score;
            }
        }
        physicalDevice = bestDevice;
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

        vk::DeviceQueueCreateInfo queueCreateInfo = {};
        queueCreateInfo.sType = vk::StructureType::eDeviceQueueCreateInfo;
        queueCreateInfo.pNext = VK_NULL_HANDLE;
        queueCreateInfo.flags = vk::DeviceQueueCreateFlags();
        queueCreateInfo.queueFamilyIndex = graphicsQueueFamily.value();
        queueCreateInfo.queueCount = 1;
        float32 queuePriority = 1.0f;
        queueCreateInfo.pQueuePriorities = &queuePriority;

        vk::DeviceCreateInfo deviceCreateInfo = {};
        deviceCreateInfo.sType = vk::StructureType::eDeviceCreateInfo;
        deviceCreateInfo.pNext = nullptr;
        deviceCreateInfo.flags = vk::DeviceCreateFlagBits();
        deviceCreateInfo.queueCreateInfoCount = 1;
        deviceCreateInfo.pQueueCreateInfos = &queueCreateInfo;
        deviceCreateInfo.enabledLayerCount = useValidationLayers ? validationLayers.size() : 0;
        deviceCreateInfo.ppEnabledLayerNames = useValidationLayers ? validationLayers.data() : nullptr;
        deviceCreateInfo.enabledExtensionCount = 0;
        deviceCreateInfo.ppEnabledExtensionNames = nullptr;
        deviceCreateInfo.pEnabledFeatures = &physicalDeviceFeatures;

        VkCall(physicalDevice.createDevice(&deviceCreateInfo, VkAllocator::Get().callbacks, &device),
               "Failed to Create Logical Device!");

        device.getQueue(graphicsQueueFamily.value(), 0, &graphicsQueue);
    }

    vk::Bool32 VkDevice::IsDeviceSuitable(vk::PhysicalDevice physicalDevice)
    {
        vk::PhysicalDeviceFeatures deviceFeatures;
        physicalDevice.getFeatures(&deviceFeatures);

        return deviceFeatures.geometryShader;
    }
}

#pragma clang diagnostic pop