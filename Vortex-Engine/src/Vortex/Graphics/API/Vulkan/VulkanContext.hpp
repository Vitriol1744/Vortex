//
// Created by Vitriol1744 on 14.09.2022.
//
#ifndef VORTEX_VULKANCONTEXT_HPP
#define VORTEX_VULKANCONTEXT_HPP

#include "Vortex/Graphics/API/Vulkan/VulkanCommon.hpp"

#include "Vortex/Core/Events/EventSystem.hpp"
#include "Vortex/Core/Events/VortexEvents.hpp"
#include "Vortex/Core/Events/IListener.hpp"

#include "Vortex/Graphics/API/IGraphicsContext.hpp"
#include "Vortex/Graphics/API/Vulkan/VulkanFramebuffer.hpp"
#include "Vortex/Graphics/API/Vulkan/VulkanGraphicsPipeline.hpp"
#include "Vortex/Graphics/API/Vulkan/VulkanShader.hpp"
#include "Vortex/Graphics/API/Vulkan/VulkanSurface.hpp"
#include "Vortex/Graphics/API/Vulkan/VulkanSwapChain.hpp"

namespace Vortex::Graphics
{
    class VT_API VulkanContext : public IGraphicsContext, public IListener
    {
        public:
            VulkanContext(uintptr_t windowHandle, uint32 windowWidth, uint32 windowHeight);
            ~VulkanContext();

            virtual inline void Present() override;
            virtual inline void Activate() override { }

            virtual bool OnEvent(IEvent& e) override;

            inline VulkanSurface& GetSurface() { return surface; }
            inline VulkanSwapChain& GetSwapChain() { return swapChain; }
            inline VulkanFramebuffer* GetBoundFramebuffer() { return boundFramebuffer; }

            void SetGraphicsPipeline(VulkanGraphicsPipeline* pipeline);
            void SetFramebuffer(VulkanFramebuffer* framebuffer);
            void AttachGraphicsPipeline(VulkanGraphicsPipeline* pipeline, GraphicsPipelineCreateInfo& createInfo);
            void DetachGraphicsPipeline(VulkanGraphicsPipeline* pipeline);
            void AttachFramebuffer(VulkanFramebuffer* framebuffer, VkRenderPass renderPass);
            void DetachFramebuffer(VulkanFramebuffer* framebuffer);

            void RecreateSwapChain();

        private:
            GLFWwindow* windowHandle;
            VulkanSurface surface;
            VulkanSwapChain swapChain;
            VulkanGraphicsPipeline* boundGraphicsPipeline = nullptr;
            VulkanFramebuffer* boundFramebuffer = nullptr;

            std::vector<VulkanGraphicsPipeline*> graphicsPipelines;
            std::vector<VulkanFramebuffer*> framebuffers;
    };
}
#endif //VORTEX_VULKANCONTEXT_HPP
