//
// Created by Vitriol1744 on 08.09.2022.
//
#define VT_EXPORT_WIN32_HEADERS
#include "VulkanInstance.hpp"

#ifdef VT_PLATFORM_WINDOWS
#include <Windows.h>
#include <vulkan/vulkan_win32.h>
#define VT_VULKAN_PLATFORM_SURFACE_EXTENSION_NAME VK_KHR_WIN32_SURFACE_EXTENSION_NAME
#endif

#include "Vortex/Graphics/API/Vulkan/VulkanRendererAPI.hpp"

//#include <volk.h>

namespace Vortex::Graphics
{
    void VulkanInstance::Create(strview appName, uint version_Major, uint version_Minor, uint version_Patch, bool useValidationLayers)
    {
        //VT_CORE_SLOW_ASSERT(volkInitialize() == VK_SUCCESS);

        VTCoreLogTrace("Vulkan: Creating Instance...");
        VkBool32 validationLayersSupported = ValidationLayersSupported();
        if (useValidationLayers && !validationLayersSupported) VTCoreLogError("Vulkan: Validation Layers Requested, but not available!");
        VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo = GetMessengerCreateInfo();
        VkValidationFeatureEnableEXT enabledFeatures = VK_VALIDATION_FEATURE_ENABLE_BEST_PRACTICES_EXT;

        VkValidationFeaturesEXT features = {};
        features.sType = VK_STRUCTURE_TYPE_VALIDATION_FEATURES_EXT;
        features.pNext = VK_NULL_HANDLE;
        features.disabledValidationFeatureCount = 0;
        features.pDisabledValidationFeatures = VK_NULL_HANDLE;
        features.enabledValidationFeatureCount = 1;
        features.pEnabledValidationFeatures = &enabledFeatures;
        debugCreateInfo.pNext = reinterpret_cast<void*>(&features);

        uint32 vortexVersion = VK_MAKE_API_VERSION(0, vortexVersion_Major, vortexVersion_Minor, vortexVersion_Patch);

        VkApplicationInfo appInfo = {};
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.pNext = VK_NULL_HANDLE;
        appInfo.pApplicationName = appName.data();
        appInfo.applicationVersion = VK_MAKE_API_VERSION(0, version_Major, version_Minor, version_Patch);
        appInfo.pEngineName = "Vortex Engine";
        appInfo.engineVersion = vortexVersion;
        appInfo.apiVersion = VK_API_VERSION_1_1;
        VTCoreLogInfo("Vulkan: API Version: {}.{}", 1, 1);

        const char* extensions[] =
        {
            VK_KHR_SURFACE_EXTENSION_NAME,
            VT_VULKAN_PLATFORM_SURFACE_EXTENSION_NAME,
            VK_EXT_DEBUG_UTILS_EXTENSION_NAME
        };

        VkInstanceCreateInfo instanceCreateInfo;
        const std::vector<const char*>& validationLayers = VulkanRendererAPI::GetValidationLayers();

        instanceCreateInfo.sType                    = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        instanceCreateInfo.pNext                    = validationLayersSupported ? reinterpret_cast<VkDebugUtilsMessengerCreateInfoEXT*>(&debugCreateInfo)
                : nullptr;
        instanceCreateInfo.flags                    = 0;
        instanceCreateInfo.pApplicationInfo         = &appInfo;
        instanceCreateInfo.enabledLayerCount        = validationLayersSupported ? validationLayers.size() : 0;
        instanceCreateInfo.ppEnabledLayerNames      = validationLayersSupported ? validationLayers.data() : nullptr;
        instanceCreateInfo.enabledExtensionCount    = std::size(extensions);
        instanceCreateInfo.ppEnabledExtensionNames  = extensions;

        VkCall_msg(vkCreateInstance(&instanceCreateInfo, VulkanAllocator::Get().callbacks, &instance), "Failed to Create VkInstance!");

        if (instance) VTCoreLogTrace("Vulkan: Instance Created Successfully!");
        VTCoreLogTrace("Vulkan: Setting up Vulkan Debug Messenger...");
        if (useValidationLayers && validationLayersSupported) SetupDebugMessenger();

        //TODO: Vulkan: Use Volk
        //volkLoadInstanceOnly(instance);
    }

    VkBool32 VulkanInstance::ValidationLayersSupported()
    {
        const std::vector<const char*>& validationLayers = VulkanRendererAPI::GetValidationLayers();

        uint32 layersCount = 0;
        VkTry_msg(vkEnumerateInstanceLayerProperties(&layersCount, nullptr), "Failed to Enumerate through instanceLayerProperties");

        std::vector<VkLayerProperties> availableLayers(layersCount);
        VkTry_msg(vkEnumerateInstanceLayerProperties(&layersCount, availableLayers.data()), "Failed to Enumerate through instanceLayerProperties");

        VTCoreLogTrace("Vulkan: Searching for validation layers...");
        for (const char* layerName : validationLayers)
        {
            bool layerFound = false;
            VTCoreLogTrace("Vulkan: Searching for {} layer", layerName);
            for (const auto& layerProperties : availableLayers)
            {
                if (!strcmp(layerName, layerProperties.layerName))
                {
                    layerFound = true;
                    VTCoreLogTrace("Vulkan: Layer {} found!", layerName);
                    break;
                }
            }
            if (!layerFound)
            {
                VTCoreLogError("Vulkan: Failed to find {} layer", layerName);
                return VK_FALSE;
            }
        }

        return VK_TRUE;
    }

    void VulkanInstance::SetupDebugMessenger()
    {
        VkDebugUtilsMessengerCreateInfoEXT createInfo = GetMessengerCreateInfo();

        VkTry_msg(CreateDebugUtilsMessengerEXT(instance, &createInfo, VulkanAllocator::Get().callbacks, &debugMessenger),
               "Failed to Create Debug Messenger!");

        if (debugMessenger) VTCoreLogTrace("Vulkan: Debug Messenger Created Successfully!");
    }
    VkDebugUtilsMessengerCreateInfoEXT VulkanInstance::GetMessengerCreateInfo()
    {
        VkDebugUtilsMessengerCreateInfoEXT messengerCreateInfo;
        messengerCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        messengerCreateInfo.pNext = nullptr;
        messengerCreateInfo.flags = 0;
        messengerCreateInfo.messageSeverity =
                VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        messengerCreateInfo.messageType =
                VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        messengerCreateInfo.pfnUserCallback = VulkanRendererAPI::ErrorCallback;

        return messengerCreateInfo;
    }

    void VulkanInstance::LoadDevice(VkDevice device) { }

    VkResult VulkanInstance::CreateDebugUtilsMessengerEXT(VkInstance instance,
                                                          const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo,
                                                          const VkAllocationCallbacks *pAllocator,
                                                          VkDebugUtilsMessengerEXT *pDebugMessenger)
    {
        static auto func = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT"));
        if (func != nullptr) return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
        else return VK_ERROR_EXTENSION_NOT_PRESENT;
    }

    void VulkanInstance::DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger,
                                                       const VkAllocationCallbacks *pAllocator)
    {
        static auto func = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT"));
        if (func != nullptr) func(instance, debugMessenger, pAllocator);
    }
}