//
// Created by Vitriol1744 on 08.09.2022.
//
#ifndef VORTEX_VULKANRENDERERAPI_HPP
#define VORTEX_VULKANRENDERERAPI_HPP

#include "Vortex/Graphics/API/Vulkan/VulkanCommon.hpp"

#include "Vortex/Graphics/API/IRendererAPI.hpp"

#include "Vortex/Graphics/API/Vulkan/VulkanInstance.hpp"
#include "Vortex/Graphics/API/Vulkan/VulkanDevice.hpp"

namespace Vortex::Graphics
{
    class VT_API VulkanRendererAPI : public IRendererAPI
    {
        public:
            virtual void InitializeImpl() override;

            virtual void BeginRenderPassImpl() override;
            virtual void EndRenderPassImpl() override;

            virtual void DrawImpl(Ref<IVertexBuffer> vertexBuffer) override;
            virtual void DrawIndexedImpl(Ref<IVertexBuffer> vertexBuffer, Ref<IIndexBuffer> indexBuffer, uint16 indexCount) override;

            virtual void SetClearColorImpl(Color color) override;
            virtual void SetViewportImpl(const Math::Rectangle& rect) override;

            virtual void ClearImpl() noexcept override;
            //virtual void DrawIndexedImpl(const Ref<IVertexArray>& mesh, uint32_t indicesCount) override;

            inline static std::string GetApplicationName() { return vkRendererAPI_Instance->applicationName; }
            inline static Math::Vec3 GetApplicationVersion() { return vkRendererAPI_Instance->applicationVersion; }

            inline static VulkanRendererAPI* Get() { return vkRendererAPI_Instance; }
            inline static constexpr const VkBool32 UseValidationLayers()
            {
                #ifdef VT_DEBUG
                constexpr const VkBool32 useValidationLayers = VK_TRUE;
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
            inline static VulkanInstance& GetVulkanInstance() { return vkRendererAPI_Instance->vulkanInstance; }
            inline static VulkanDevice& GetDevice() { return vkRendererAPI_Instance->device; }

            static VKAPI_ATTR VkBool32 VKAPI_CALL ErrorCallback(VkDebugUtilsMessageSeverityFlagBitsEXT severity,
                            VkDebugUtilsMessageTypeFlagsEXT type,
                            const VkDebugUtilsMessengerCallbackDataEXT* data,
                            void* userData);

            void CreateCommandPool();
            void DestroyCommandPool();
            void CreateCommandBuffers();
            void CreateSyncObjects();
            void DestroySyncObjects();

            inline static VkCommandPool GetCommandPool() noexcept { return vkRendererAPI_Instance->commandPool; }
            inline static VkCommandBuffer GetCommandBuffer() noexcept { return vkRendererAPI_Instance->commandBuffers[vkRendererAPI_Instance->currentFrameIndex]; }

            inline static VkSemaphore GetRenderFinishedSemaphore() noexcept
            {
                return vkRendererAPI_Instance->renderFinishedSemaphores[vkRendererAPI_Instance->currentFrameIndex];
            }
            inline static uint32 GetCurrentFrameIndex() noexcept { return vkRendererAPI_Instance->currentFrameIndex; }

        private:
            static VulkanRendererAPI* vkRendererAPI_Instance;
            Color clearColor = { 0.0f, 0.0f, 0.0f, 1.0f };

            VulkanInstance vulkanInstance;
            VulkanDevice device;

            static constexpr const uint8 MAX_FRAMES_IN_FLIGHT = 2;
            VkCommandPool commandPool;
            std::vector<VkCommandBuffer> commandBuffers;

            std::vector<VkSemaphore> imageAvailableSemaphores;
            std::vector<VkSemaphore> renderFinishedSemaphores;
            std::vector<VkFence> inFlightFences;
            uint32 currentFrameIndex = 0;
    };
}
#endif //VORTEX_VULKANRENDERERAPI_HPP
