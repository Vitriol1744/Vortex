/*
 * Created by v1tr10l7 on 18.07.2024.
 * Copyright (c) 2024-2026, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#include <Vortex/Renderer/RHI/Vulkan/VulkanInstance.hpp>

#include <Vortex/Core/Platform.hpp>
#include <Vortex/Window/Window.hpp>

#include <cstring>

import Prism.Containers.Vector;
import Prism.String.Formatter;

// Storage of the default dynamic dispatcher (VULKAN_HPP_DISPATCH_LOADER_DYNAMIC);
// one definition for the whole backend library.
VULKAN_HPP_DEFAULT_DISPATCH_LOADER_DYNAMIC_STORAGE

namespace Vortex::Vulkan
{
    namespace
    {
        constexpr const char* kValidationLayer = "VK_LAYER_KHRONOS_validation";

        VKAPI_ATTR VkBool32 VKAPI_CALL
        DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT      severity,
                      VkDebugUtilsMessageTypeFlagsEXT             type,
                      const VkDebugUtilsMessengerCallbackDataEXT* data,
                      void*                                       userData)
        {
            (void)type;
            (void)userData;
            switch (severity)
            {
                case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
                    VtCoreErrorTag("Vulkan", "{}", data->pMessage);
                    break;
                case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
                    VtCoreWarnTag("Vulkan", "{}", data->pMessage);
                    break;
                case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
                    VtCoreInfoTag("Vulkan", "{}", data->pMessage);
                    break;
                default: VtCoreTraceTag("Vulkan", "{}", data->pMessage); break;
            }
            return VK_FALSE;
        }

        vk::DebugUtilsMessengerCreateInfoEXT MessengerInfo()
        {
            vk::DebugUtilsMessengerCreateInfoEXT info{};
            info.messageSeverity
                = vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning
                | vk::DebugUtilsMessageSeverityFlagBitsEXT::eError;
            info.messageType = vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral
                             | vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation
                             | vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance;
            info.pfnUserCallback
                = reinterpret_cast<vk::PFN_DebugUtilsMessengerCallbackEXT>(
                    DebugCallback);
            return info;
        }

        bool HasLayer(const char* name)
        {
            u32 count = 0;
            VkCall(vk::enumerateInstanceLayerProperties(&count, nullptr));
            Vector<vk::LayerProperties> layers(count);
            VkCall(vk::enumerateInstanceLayerProperties(&count, layers.Raw()));
            for (const auto& layer : layers)
                if (std::strcmp(layer.layerName, name) == 0) return true;
            return false;
        }
        bool HasExtension(const Vector<vk::ExtensionProperties>& available,
                          const char*                            name)
        {
            for (const auto& extension : available)
                if (std::strcmp(extension.extensionName, name) == 0)
                    return true;
            return false;
        }

        String CString(StringView view) { return String(view); }
    } // namespace

    Expected<void, String>
    VulkanInstance::Initialize(StringView     applicationName,
                               const Version& applicationVersion,
                               StringView     engineName,
                               const Version& engineVersion, bool validation)
    {
        VtCoreTrace("Vulkan: Creating instance...");

        // Bootstrap the dynamic dispatcher with the loader's entry point.
        static vk::detail::DynamicLoader loader;
        auto                             getInstanceProcAddr
            = loader.getProcAddress<PFN_vkGetInstanceProcAddr>(
                "vkGetInstanceProcAddr");
        if (!getInstanceProcAddr)
            return Unexpected(String("Vulkan: Failed to load the Vulkan loader"));
        VULKAN_HPP_DEFAULT_DISPATCHER.init(getInstanceProcAddr);

        u32 version = VK_API_VERSION_1_0;
        if (VULKAN_HPP_DEFAULT_DISPATCHER.vkEnumerateInstanceVersion)
            VkCall(vk::enumerateInstanceVersion(&version));
        VtCoreInfo("Vulkan: Loader API version {}.{}.{}",
                   VK_API_VERSION_MAJOR(version), VK_API_VERSION_MINOR(version),
                   VK_API_VERSION_PATCH(version));
        if (version < kRequiredApiVersion)
            return Unexpected(String("Vulkan: Vulkan 1.3 is required"));

        u32 extensionCount = 0;
        VkCall(vk::enumerateInstanceExtensionProperties(nullptr, &extensionCount,
                                                        nullptr));
        Vector<vk::ExtensionProperties> available(extensionCount);
        VkCall(vk::enumerateInstanceExtensionProperties(nullptr, &extensionCount,
                                                        available.Raw()));

        // Platform surface extensions by name: their macros live in the
        // per-platform Vulkan headers, which drag in X11/Wayland/Win32.
        Vector<const char*> extensions = {VK_KHR_SURFACE_EXTENSION_NAME};
#if defined(VT_PLATFORM_ANDROID)
        extensions.PushBack("VK_KHR_android_surface");
#elif defined(VT_PLATFORM_LINUX)
        if (Window::WindowSubsystem() == WindowSubsystem::eWayland)
            extensions.PushBack("VK_KHR_wayland_surface");
        else extensions.PushBack("VK_KHR_xcb_surface");
#elif defined(VT_PLATFORM_WINDOWS)
        extensions.PushBack("VK_KHR_win32_surface");
#endif
        for (const char* required : extensions)
            if (!HasExtension(available, required))
                return Unexpected(Prism::Format(
                    "Vulkan: Missing instance extension '{}'", required));

        // Debug names and labels work without the validation layers (RenderDoc
        // and other tools use them too), so enable the extension when present.
        m_DebugUtils = HasExtension(available, VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        if (m_DebugUtils) extensions.PushBack(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

        m_Validation = validation && m_DebugUtils && HasLayer(kValidationLayer);
        if (validation && !m_Validation)
            VtCoreWarn(
                "Vulkan: Validation requested but the Khronos validation layer "
                "or VK_EXT_debug_utils is unavailable");

        String              appName    = CString(applicationName);
        String              engineNameZ = CString(engineName);
        vk::ApplicationInfo appInfo{};
        appInfo.pApplicationName   = appName.Raw();
        appInfo.applicationVersion = VK_MAKE_API_VERSION(
            0, applicationVersion.Major, applicationVersion.Minor,
            applicationVersion.Patch);
        appInfo.pEngineName   = engineNameZ.Raw();
        appInfo.engineVersion = VK_MAKE_API_VERSION(
            0, engineVersion.Major, engineVersion.Minor, engineVersion.Patch);
        appInfo.apiVersion = kRequiredApiVersion;

        auto                   messengerInfo = MessengerInfo();
        vk::InstanceCreateInfo instanceInfo{};
        instanceInfo.pApplicationInfo        = &appInfo;
        instanceInfo.enabledExtensionCount   = extensions.Size();
        instanceInfo.ppEnabledExtensionNames = extensions.Raw();
        if (m_Validation)
        {
            instanceInfo.enabledLayerCount   = 1;
            instanceInfo.ppEnabledLayerNames = &kValidationLayer;
            // Also reports problems during vkCreateInstance itself.
            instanceInfo.pNext = &messengerInfo;
        }

        vk::Result result
            = vk::createInstance(&instanceInfo, nullptr, &m_Instance);
        if (result != vk::Result::eSuccess)
            return Unexpected(Prism::Format("Vulkan: vkCreateInstance failed: {}",
                                            VkResultToString(result)));
        VULKAN_HPP_DEFAULT_DISPATCHER.init(m_Instance);

        if (m_Validation)
            VkCall(m_Instance.createDebugUtilsMessengerEXT(
                &messengerInfo, nullptr, &m_DebugMessenger));

        VtCoreTrace("Vulkan: Instance created (validation: {})", m_Validation);
        return {};
    }

    void VulkanInstance::Destroy()
    {
        if (!m_Instance) return;
        VtCoreTrace("Vulkan: Destroying instance...");
        if (m_DebugMessenger)
            m_Instance.destroyDebugUtilsMessengerEXT(m_DebugMessenger, nullptr);
        m_Instance.destroy(nullptr);
        m_DebugMessenger = VK_NULL_HANDLE;
        m_Instance       = VK_NULL_HANDLE;
    }
} // namespace Vortex::Vulkan
