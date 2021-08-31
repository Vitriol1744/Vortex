//
// Created by vitriol1744 on 29.08.2021.
//
#pragma once

#include "Vortex/Core/Core.hpp"

#include "Vortex/Graphics/API/IRendererAPI.hpp"

#include "Vortex/Graphics/API/Vulkan/VkCommon.hpp"
#include "Vortex/Graphics/API/Vulkan/VkInstance.hpp"
#include "Vortex/Graphics/API/Vulkan/VkDevice.hpp"

namespace Vortex::Graphics
{
    class VT_API VkRendererAPI : public IRendererAPI
    {
        public:
            virtual void InitializeImpl() override;

            virtual void SetClearColorImpl(Math::Vec4 color) override;
            virtual void SetViewportImpl(const Math::Rectangle& rect) override;

            virtual void ClearImpl() noexcept override;
            virtual void DrawIndexedImpl(const Ref<IVertexArray>& mesh, uint32_t indicesCount) override;

            inline static std::string GetApplicationName() { return vkRendererAPI_Instance->applicationName; }
            inline static Math::Vec3 GetApplicationVersion() { return vkRendererAPI_Instance->applicationVersion; }

            inline static constexpr const vk::Bool32 UseValidationLayers()
            {
                #ifdef VT_DEBUG
                    constexpr const vk::Bool32 useValidationLayers = VK_TRUE;
                #else
                    constexpr const vk::Bool32 useValidationLayers = VK_FALSE;
                #endif

                return useValidationLayers;
            }
            inline static const std::vector<const char*>& GetValidationLayers()
            {
                static const std::vector<const char*> validationLayers = { "VK_LAYER_KHRONOS_validation" };

                return validationLayers;
            }
            inline static VkInstance& GetVulkanInstance() { return vkRendererAPI_Instance->vkInstance; }
            inline static VkDevice& GetDevice() { return vkRendererAPI_Instance->device; }

            static VKAPI_ATTR VkBool32 VKAPI_CALL ErrorCallback
            (
                VkDebugUtilsMessageSeverityFlagBitsEXT severity,
                VkDebugUtilsMessageTypeFlagsEXT type,
                const VkDebugUtilsMessengerCallbackDataEXT* data,
                void* userData
            );

        private:
            static VkRendererAPI* vkRendererAPI_Instance;

            VkInstance vkInstance;
            VkDevice device;
    };
}

