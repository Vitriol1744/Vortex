//
// Created by vitriol1744 on 29.08.2021.
//
#include "VkRendererAPI.hpp"

namespace Vortex::Graphics
{
    VkRendererAPI* VkRendererAPI::vkRendererAPI_Instance = nullptr;

    void VkRendererAPI::InitializeImpl()
    {
        vkRendererAPI_Instance = dynamic_cast<VkRendererAPI*>(instance);
        VT_CORE_ASSERT(vkRendererAPI_Instance != nullptr);

        uint32 appVersionMajor = applicationVersion.x;
        uint32 appVersionMinor = applicationVersion.y;
        uint32 appVersionPatch = applicationVersion.z;

        vkInstance.Create(applicationName, appVersionMajor, appVersionMinor, appVersionPatch, UseValidationLayers());
    }

    void VkRendererAPI::SetClearColorImpl(Math::Vec4 color)
    {
        //TODO: Vulkan: SetClearColorImpl!
    }
    void VkRendererAPI::SetViewportImpl(const Math::Rectangle& rect)
    {
        //TODO: Vulkan: SetViewportImpl!
    }

    void VkRendererAPI::ClearImpl() noexcept
    {
        //TODO: Vulkan: ClearImpl!
    }
    void VkRendererAPI::DrawIndexedImpl(const Ref<IVertexArray>& mesh, uint32_t indicesCount)
    {
        //TODO: Vulkan: DrawIndexedImpl!
    }

    VKAPI_ATTR VkBool32 VKAPI_CALL VkRendererAPI::ErrorCallback
    (
            VkDebugUtilsMessageSeverityFlagBitsEXT severity,
            VkDebugUtilsMessageTypeFlagsEXT type,
            const VkDebugUtilsMessengerCallbackDataEXT* data,
            void* userData
    )
    {
        switch (severity)
        {
            case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
                VTCoreLogTrace("Vulkan Validation Layer: {}", data->pMessage);
                break;
            case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
                VTCoreLogInfo("Vulkan Validation Layer: {}", data->pMessage);
                break;
            case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
                VTCoreLogWarn("Vulkan Validation Layer: {}", data->pMessage);
                break;
            case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
                VTCoreLogError("Vulkan Validation Layer: {}", data->pMessage);
                break;

            default:
                break;
        }

        return VK_FALSE;
    }
}