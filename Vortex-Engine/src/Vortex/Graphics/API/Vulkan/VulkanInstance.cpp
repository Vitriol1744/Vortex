/*
 * Created by v1tr10l7 on 18.07.2024.
 * Copyright (c) 2024-2024, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#include "vtpch.hpp"

#include "Vortex/Core/Core.hpp"
#include "Vortex/Engine/Application.hpp"
#include "Vortex/Graphics/API/Vulkan/VulkanInstance.hpp"

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
static constexpr const vk::Bool32 s_UseValidationLayers = true;
#else
static constexpr const vk::Bool32 s_UseValidationLayers = false;
#endif

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

    VulkanInstance::VulkanInstance()
    {
        vk::DynamicLoader         dl;
        PFN_vkGetInstanceProcAddr vkGetInstanceProcAddr
            = dl.getProcAddress<PFN_vkGetInstanceProcAddr>(
                "vkGetInstanceProcAddr");
        VULKAN_HPP_DEFAULT_DISPATCHER.init(vkGetInstanceProcAddr);

        vk::Bool32 validationLayersSupported = ValidationLayersSupported();
        if (s_UseValidationLayers && !validationLayersSupported)
            VtCoreError("Validation Layers Requested, but not available!");
        VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo
            = GetDebugMessengerCreateInfo();

        Version             vortexVersion = Vortex::g_Version;
        const Version&      appVersion    = Application::Get()->GetVersion();

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
            = s_UseValidationLayers ? s_ValidationLayers.size() : 0;
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

    VulkanInstance::~VulkanInstance()
    {
        auto dldi
            = vk::DispatchLoaderDynamic(m_Instance, vkGetInstanceProcAddr);
        m_Instance.destroyDebugUtilsMessengerEXT(m_DebugMessenger, nullptr,
                                                 dldi);
        m_Instance.destroy(nullptr, dldi);
    }

    const std::vector<const char*>& VulkanInstance::GetValidationLayers()
    {
        return s_ValidationLayers;
    }
    vk::Bool32 VulkanInstance::ShouldUseValidationLayers()
    {
        return s_UseValidationLayers;
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
