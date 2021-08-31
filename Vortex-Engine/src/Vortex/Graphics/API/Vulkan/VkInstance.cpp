//
// Created by vitriol1744 on 27.08.2021.
//
#include "VkInstance.hpp"

#include "Vortex/Graphics/API/Vulkan/VkRendererAPI.hpp"

#include <array>

#ifdef VT_PLATFORM_WINDOWS
    #include <Windows.h>
    #include <vulkan/vulkan_win32.h>
    #define VT_VULKAN_PLATFORM_SURFACE_EXTENSION_NAME VK_KHR_WIN32_SURFACE_EXTENSION_NAME
#elif defined(VT_PLATFORM_LINUX)
    #include <X11/Xlib.h>
    #include <vulkan/vulkan_xlib.h>
    #define VT_VULKAN_PLATFORM_SURFACE_EXTENSION_NAME VK_KHR_XLIB_SURFACE_EXTENSION_NAME
#endif

VULKAN_HPP_DEFAULT_DISPATCH_LOADER_DYNAMIC_STORAGE

namespace Vortex::Graphics
{
    void VkInstance::Create(std::string_view appName, uint version_Major, uint version_Minor, uint version_Patch, bool useValidationLayers)
    {
        vk::DynamicLoader dl;
        PFN_vkGetInstanceProcAddr vkGetInstanceProcAddr = dl.getProcAddress<PFN_vkGetInstanceProcAddr>("vkGetInstanceProcAddr");
        VULKAN_HPP_DEFAULT_DISPATCHER.init(vkGetInstanceProcAddr);

        vk::Bool32 validationLayersSupported = ValidationLayersSupported();
        if (useValidationLayers && !validationLayersSupported) VTCoreLogError("Validation Layers Requested, but not available!");
        VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo = GetMessengerCreateInfo();

        uint32 vortexVersion = VK_MAKE_VERSION(vortexVersion_Major, vortexVersion_Minor, vortexVersion_Patch);

        vk::ApplicationInfo appInfo;
        appInfo.sType                               = vk::StructureType::eApplicationInfo;
        appInfo.pNext                               = nullptr;
        appInfo.pApplicationName                    = appName.data();
        appInfo.applicationVersion                  = VK_MAKE_VERSION(version_Major, version_Minor, version_Patch);
        appInfo.pEngineName                         = "Vortex Engine";
        appInfo.engineVersion                       = vortexVersion;
        appInfo.apiVersion                          = VK_API_VERSION_1_2;

        const char* extensions[] =
        {
            VK_KHR_SURFACE_EXTENSION_NAME,
            VT_VULKAN_PLATFORM_SURFACE_EXTENSION_NAME,
            VK_EXT_DEBUG_UTILS_EXTENSION_NAME
        };

        vk::InstanceCreateInfo instanceCreateInfo;
        const std::vector<const char*>& validationLayers = VkRendererAPI::GetValidationLayers();

        instanceCreateInfo.sType                    = vk::StructureType::eInstanceCreateInfo;
        instanceCreateInfo.pNext                    = validationLayersSupported ? (vk::DebugUtilsMessengerCreateInfoEXT*) &debugCreateInfo : nullptr;
        instanceCreateInfo.flags                    = vk::InstanceCreateFlags();
        instanceCreateInfo.pApplicationInfo         = &appInfo;
        instanceCreateInfo.enabledLayerCount        = validationLayersSupported ? validationLayers.size() : 0;
        instanceCreateInfo.ppEnabledLayerNames      = validationLayersSupported ? validationLayers.data() : nullptr;
        instanceCreateInfo.enabledExtensionCount    = std::size(extensions);
        instanceCreateInfo.ppEnabledExtensionNames  = extensions;

        VkCall(vk::createInstance(&instanceCreateInfo, VkAllocator::Get().callbacks, &instance), "Failed to Create VkInstance!");
        VULKAN_HPP_DEFAULT_DISPATCHER.init(instance);

        SetupDebugMessenger();
    }

    vk::Bool32 VkInstance::ValidationLayersSupported()
    {
        const std::vector<const char*>& validationLayers = VkRendererAPI::GetValidationLayers();

        uint32 layersCount = 0;
        VkCall(vk::enumerateInstanceLayerProperties(&layersCount, nullptr), "Failed to Enumerate through instanceLayerProperties");

        std::vector<vk::LayerProperties> availableLayers(layersCount);
        VkCall(vk::enumerateInstanceLayerProperties(&layersCount, availableLayers.data()), "Failed to Enumerate through instanceLayerProperties");

        for (const char* layerName : validationLayers)
        {
            bool layerFound = false;

            for (const auto& layerProperties : availableLayers)
            {
                if (!strcmp(layerName, layerProperties.layerName))
                {
                    layerFound = true;
                    break;
                }
            }
            if (!layerFound)
            {
                VTCoreLogError("Failed to find {} layer", layerName);
                return VK_FALSE;
            }
        }

        return VK_TRUE;
    }

    void VkInstance::SetupDebugMessenger()
    {
        vk::DebugUtilsMessengerCreateInfoEXT createInfo = GetMessengerCreateInfo();

        VkCall(instance.createDebugUtilsMessengerEXT(&createInfo, VkAllocator::Get().callbacks, &debugMessenger),
            "Failed to Create Debug Messenger!");
    }

    vk::DebugUtilsMessengerCreateInfoEXT VkInstance::GetMessengerCreateInfo()
    {
        vk::DebugUtilsMessengerCreateInfoEXT messengerCreateInfo;
        messengerCreateInfo.sType = vk::StructureType::eDebugUtilsMessengerCreateInfoEXT;
        messengerCreateInfo.pNext = nullptr;
        messengerCreateInfo.flags = vk::DebugUtilsMessengerCreateFlagBitsEXT();
        messengerCreateInfo.messageSeverity =
                vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning |
                vk::DebugUtilsMessageSeverityFlagBitsEXT::eError;
        messengerCreateInfo.messageType =
                vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral |
                vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation |
                vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance;
        messengerCreateInfo.pfnUserCallback = VkRendererAPI::ErrorCallback;

        return messengerCreateInfo;
    }
}