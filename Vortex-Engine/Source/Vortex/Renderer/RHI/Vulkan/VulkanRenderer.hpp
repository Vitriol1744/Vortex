/*
 * Created by v1tr10l7 on 13.08.2024.
 * Copyright (c) 2024-2024, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include <Vortex/Renderer/RHI/RendererBackend.hpp>
#include <Vortex/Renderer/RHI/Vulkan/VulkanDevice.hpp>
#include <Vortex/Renderer/RHI/Vulkan/VulkanInstance.hpp>
#include <Vortex/Renderer/RHI/Vulkan/VulkanSwapChain.hpp>

namespace Vortex
{
    class VT_RENDERER_RHI_VULKAN_API VulkanRenderer : public RendererBackend
    {
      public:
        VulkanRenderer();
        virtual ~VulkanRenderer() = default;

        virtual u32 CurrentFrameIndex() override
        {
            return m_CurrentSwapChain->CurrentFrameIndex();
        }

        virtual void Initialize(const RHI::Specification& specs) override;
        virtual void Shutdown() override;

        virtual ::Ref<Framebuffer>
        CreateFramebuffer(const FramebufferSpecification& spec) override;
        virtual ::Ref<GraphicsPipeline> CreateGraphicsPipeline(
            const GraphicsPipelineSpecification& spec) override;
        virtual ::Ref<VertexBuffer>  CreateVertexBuffer(void* data,
                                                        usize size) override;
        virtual ::Ref<IndexBuffer>   CreateIndexBuffer(u32*  data,
                                                       usize size) override;
        virtual ::Ref<UniformBuffer> CreateUniformBuffer(usize size) override;
        virtual ::Ref<SwapChain>     CreateSwapChain(class Window* window,
                                                     bool vsync = false) override;
        virtual ::Ref<Shader>        CreateShader(PathView path) override;
        virtual ::Ref<Shader>        CreateShader(PathView vertexPath,
                                                  PathView fragmentPath) override;
        virtual ::Ref<Material>
        CreateMaterial(const ::Ref<Shader> shader) override;
        virtual ::Ref<Texture2D> CreateTexture2D(ImageView image) override;

        virtual void  BeginFrame(::Ref<SwapChain> swapChain) override;
        virtual void  EndFrame() override;
        virtual void  BeginRenderPass(::Ref<Framebuffer> target) override;
        virtual void  EndRenderPass() override;

        virtual void  WaitIdle() override;

        virtual void  Draw(::Ref<GraphicsPipeline> pipeline,
                           ::Ref<VertexBuffer>     vertexBuffer,
                           ::Ref<IndexBuffer> indexBuffer, u32 indexCount,
                           ::Ref<Material> material) override;

        virtual usize MemoryUsage() override;
        virtual usize MemoryBudget() override;

        inline static const class VulkanInstance& VulkanInstance()
        {
            VtCoreAssert(s_VkInstance);
            return s_VkInstance;
        }
        inline static consteval const VulkanPhysicalDevice& PhysicalDevice()
        {
            return s_PhysicalDevice;
        }
        inline static consteval const VulkanDevice& Device()
        {
            return s_Device;
        }

      private:
        ::Ref<VulkanSwapChain>              m_CurrentSwapChain = nullptr;
        vk::PhysicalDeviceMemoryProperties2 m_MemoryProperties{};
        vk::PhysicalDeviceMemoryBudgetPropertiesEXT m_MemoryBudgetProperties{};

        static class VulkanInstance                 s_VkInstance;
        static VulkanPhysicalDevice                 s_PhysicalDevice;
        static VulkanDevice                         s_Device;
    };
}; // namespace Vortex
