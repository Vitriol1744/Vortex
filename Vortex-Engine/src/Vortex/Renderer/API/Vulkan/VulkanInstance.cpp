/*
 * Created by v1tr10l7 on 18.07.2024.
 * Copyright (c) 2024-2024, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#include "vtpch.hpp"

#include "Vortex/Core/Core.hpp"
#include "Vortex/Engine/Application.hpp"
#include "Vortex/Renderer/API/Vulkan/VulkanInstance.hpp"
#include <vulkan/vulkan_core.h>

#ifdef VT_PLATFORM_WINDOWS
    #include <Windows.h>
    #include <vulkan/vulkan_win32.h>
    #define VT_VULKAN_PLATFORM_SURFACE_EXTENSION_NAME                          \
        VK_KHR_WIN32_SURFACE_EXTENSION_NAME
#elifdef VT_PLATFORM_LINUX
    #include <X11/Xlib.h>
    #include <vulkan/vulkan_xlib.h>
    #define VT_VULKAN_PLATFORM_SURFACE_EXTENSION_NAME                          \
        VK_KHR_XLIB_SURFACE_EXTENSION_NAME
#endif

VULKAN_HPP_DEFAULT_DISPATCH_LOADER_DYNAMIC_STORAGE;

#ifndef VT_DIST
static constexpr const vk::Bool32 s_UseValidationLayers = VK_TRUE;
#else
static constexpr const vk::Bool32 s_UseValidationLayers = VK_FALSE;
#endif

using Vortex::u32;
static u32 s_MinimumVulkanVersionSupported = VK_API_VERSION_1_3;
static std::vector<const char*> s_ValidationLayers
    = {"VK_LAYER_KHRONOS_validation"};

namespace Vortex
{
    static VKAPI_ATTR VkBool32 VKAPI_CALL ErrorCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT      severity,
        VkDebugUtilsMessageTypeFlagsEXT             type,
        const VkDebugUtilsMessengerCallbackDataEXT* data, void* userData)
    {
        (void)type;
        (void)userData;
        (void)data;
        switch (severity)
        {
            case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
                VtCoreTrace("Vulkan Validation Layer: {}", data->pMessage);
                break;
            case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
                VtCoreInfo("Vulkan Validation Layer: {}", data->pMessage);
                break;
            case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
                VtCoreWarn("Vulkan Validation Layer: {}", data->pMessage);
                break;
            case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
                VtCoreError("Vulkan Validation Layer: {}", data->pMessage);
                break;

            default: break;
        }

        return VK_FALSE;
    }

    void VulkanInstance::Initialize()
    {
        VtCoreTrace("Vulkan: Creating instance...");

        {
            vk::DynamicLoader         dl;
            PFN_vkGetInstanceProcAddr vkGetInstanceProcAddr
                = dl.getProcAddress<PFN_vkGetInstanceProcAddr>(
                    "vkGetInstanceProcAddr");
            VULKAN_HPP_DEFAULT_DISPATCHER.init(vkGetInstanceProcAddr);
        }

        u32 vulkanVersion      = GetVulkanVersion();
        u32 vulkanVersionMajor = VK_VERSION_MAJOR(vulkanVersion);
        u32 vulkanVersionMinor = VK_VERSION_MINOR(vulkanVersion);
        u32 vulkanVersionPatch = VK_VERSION_PATCH(vulkanVersion);
        (void)vulkanVersion;
        (void)vulkanVersionMajor;
        (void)vulkanVersionMinor;
        (void)vulkanVersionPatch;

        VtCoreAssert(vulkanVersion >= s_MinimumVulkanVersionSupported);
        VtCoreInfo("Vulkan: API version: {}.{}.{}", vulkanVersionMajor,
                   vulkanVersionMinor, vulkanVersionPatch);

        vk::Bool32 validationLayersSupported = ValidationLayersSupported();
        if (s_UseValidationLayers && !validationLayersSupported)
            VtCoreError("Validation Layers Requested, but not available!");
        auto                debugCreateInfo = GetDebugMessengerCreateInfo();

        Version             vortexVersion   = Vortex::g_Version;
        const Version&      appVersion      = Application::Get()->GetVersion();

        vk::ApplicationInfo appInfo{};
        appInfo.sType              = vk::StructureType::eApplicationInfo;
        appInfo.pNext              = nullptr;
        appInfo.pApplicationName   = Application::Get()->GetName().data();
        appInfo.applicationVersion = VK_MAKE_VERSION(
            appVersion.Major, appVersion.Minor, appVersion.Patch);
        appInfo.pEngineName   = Vortex::g_EngineName.data();
        appInfo.engineVersion = VK_MAKE_VERSION(
            vortexVersion.Major, vortexVersion.Minor, vortexVersion.Patch);
        appInfo.apiVersion = VK_API_VERSION_1_3;
        VtCoreInfo(
            "Vulkan: Application Info: {{ applicationName: {}, "
            "applicationVersion: {}, engineName: {}, engineVersion: {}}}",
            appInfo.pApplicationName, appVersion, appInfo.pEngineName,
            vortexVersion);

        std::vector<const char*> extensions
            = {VK_KHR_SURFACE_EXTENSION_NAME,
               VT_VULKAN_PLATFORM_SURFACE_EXTENSION_NAME,
               VK_EXT_DEBUG_UTILS_EXTENSION_NAME};

        if (s_UseValidationLayers)
        {
            extensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
            extensions.push_back(
                VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
        }

        vk::InstanceCreateInfo createInfo{};
        createInfo.sType = vk::StructureType::eInstanceCreateInfo;
        createInfo.pNext
            = s_UseValidationLayers
                ? reinterpret_cast<vk::DebugUtilsMessengerCreateInfoEXT*>(
                    &debugCreateInfo)
                : nullptr;
        createInfo.flags            = vk::InstanceCreateFlags();
        createInfo.pApplicationInfo = &appInfo;
        createInfo.enabledLayerCount
            = s_UseValidationLayers ? static_cast<u32>(s_ValidationLayers.size()) : 0;
        createInfo.ppEnabledLayerNames     = s_UseValidationLayers
                                               ? s_ValidationLayers.data()
                                               : VK_NULL_HANDLE;
        createInfo.enabledExtensionCount   = extensions.size();
        createInfo.ppEnabledExtensionNames = extensions.data();

        VkCall(vk::createInstance(&createInfo, nullptr, &m_Instance));
        VULKAN_HPP_DEFAULT_DISPATCHER.init(m_Instance);

        if (m_Instance) VtCoreTrace("Vulkan: Instance created successfully");
        SetupDebugMessenger();
    }

    void VulkanInstance::Destroy()
    {
        VtCoreTrace("Vulkan: Destroying instance...");
        auto dldi
            = vk::DispatchLoaderDynamic(m_Instance, vkGetInstanceProcAddr);
        if (m_DebugMessenger)
            m_Instance.destroyDebugUtilsMessengerEXT(m_DebugMessenger, nullptr,
                                                     dldi);
        if (m_Instance) m_Instance.destroy(nullptr, dldi);

        m_DebugMessenger = VK_NULL_HANDLE;
        m_Instance       = VK_NULL_HANDLE;
    }

    const std::vector<const char*>& VulkanInstance::GetValidationLayers()
    {
        return s_ValidationLayers;
    }
    vk::Bool32 VulkanInstance::ShouldUseValidationLayers()
    {
        return s_UseValidationLayers;
    }

    u32 VulkanInstance::GetVulkanVersion()
    {
        u32 version = 0;
        VkCall(vk::enumerateInstanceVersion(&version));

        return version;
    }

    void VulkanInstance::SetupDebugMessenger()
    {
        auto dldi
            = vk::DispatchLoaderDynamic(m_Instance, vkGetInstanceProcAddr);
        vk::DebugUtilsMessengerCreateInfoEXT createInfo
            = GetDebugMessengerCreateInfo();

        VkCall(m_Instance.createDebugUtilsMessengerEXT(
            &createInfo, nullptr, &m_DebugMessenger, dldi));
        if (m_DebugMessenger)
            VtCoreTrace("Vulkan: Debug messenger created successfully");
    }

    vk::Bool32 VulkanInstance::ValidationLayersSupported()
    {
        u32 layerCount = 0;
        VkCall(vk::enumerateInstanceLayerProperties(&layerCount, nullptr));

        std::vector<vk::LayerProperties> availableLayers(layerCount);
        VkCall(vk::enumerateInstanceLayerProperties(&layerCount,
                                                    availableLayers.data()));
        for (const char* layerName : s_ValidationLayers)
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
                VtCoreError("Vulkan: Failed to find '{}' layer", layerName);
                return VK_FALSE;
            }
        }

        return VK_TRUE;
    }
    vk::DebugUtilsMessengerCreateInfoEXT
    VulkanInstance::GetDebugMessengerCreateInfo()
    {
        vk::DebugUtilsMessengerCreateInfoEXT createInfo{};
        createInfo.sType = vk::StructureType::eDebugUtilsMessengerCreateInfoEXT;
        createInfo.pNext = nullptr;
        createInfo.flags = vk::DebugUtilsMessengerCreateFlagBitsEXT();
        createInfo.messageSeverity
            = vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning
            | vk::DebugUtilsMessageSeverityFlagBitsEXT::eError;
        createInfo.messageType
            = vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral
            | vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation
            | vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance;

        createInfo.pfnUserCallback = ErrorCallback;
        return createInfo;
    }
}; // namespace Vortex
