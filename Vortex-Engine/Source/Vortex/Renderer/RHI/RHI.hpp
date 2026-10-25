/*
 * Created by v1tr10l7 on 10.07.2026.
 * Copyright (c) 2024-2026, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include <Vortex/Renderer/RHI/Framebuffer.hpp>
#include <Vortex/Renderer/RHI/GraphicsPipeline.hpp>
#include <Vortex/Renderer/RHI/IndexBuffer.hpp>
#include <Vortex/Renderer/RHI/Material.hpp>
#include <Vortex/Renderer/RHI/Shader.hpp>
#include <Vortex/Renderer/RHI/SwapChain.hpp>
#include <Vortex/Renderer/RHI/Texture2D.hpp>
#include <Vortex/Renderer/RHI/UniformBuffer.hpp>
#include <Vortex/Renderer/RHI/VertexBuffer.hpp>

namespace Vortex
{
    namespace RHI
    {
        enum class API
        {
            None = 0,
            eOpenGL,
            eDirectX11,
            eDirectX12,
            eVulkan,
            eMetal,
        };

        struct Specification
        {
            String  EngineName;
            Version EngineVersion;
            String  ApplicationName;
            Version ApplicationVersion;

            API     API               = API::eVulkan;
            usize   MaxFramesInFlight = 2;
        };

        bool VT_RENDERER_RHI_API Initialize(const Specification& specification);
        void VT_RENDERER_RHI_API Shutdown();

        const Specification& VT_RENDERER_RHI_API Configuration();

        ::Ref<Framebuffer> VT_RENDERER_RHI_API
        CreateFramebuffer(const FramebufferSpecification& spec);
        ::Ref<GraphicsPipeline> VT_RENDERER_RHI_API
        CreateGraphicsPipeline(const GraphicsPipelineSpecification& spec);
        ::Ref<VertexBuffer> VT_RENDERER_RHI_API CreateVertexBuffer(void* data,
                                                                   usize size);
        ::Ref<IndexBuffer> VT_RENDERER_RHI_API  CreateIndexBuffer(u32*  data,
                                                                  usize size);
        ::Ref<UniformBuffer>
            VT_RENDERER_RHI_API CreateUniformBuffer(usize size);
        ::Ref<SwapChain> VT_RENDERER_RHI_API
        CreateSwapChain(class Window* window, bool vsync);
        ::Ref<Shader> VT_RENDERER_RHI_API CreateShader(PathView path);
        ::Ref<Shader> VT_RENDERER_RHI_API CreateShader(PathView vertexPath,
                                                       PathView fragmentPath);
        ::Ref<Material>
            VT_RENDERER_RHI_API CreateMaterial(const ::Ref<Shader> shader);
        ::Ref<Texture2D> VT_RENDERER_RHI_API CreateTexture2D(ImageView image);

        u32 VT_RENDERER_RHI_API              CurrentFrameIndex();

        void VT_RENDERER_RHI_API  BeginFrame(Ref<SwapChain> swapChain);
        void VT_RENDERER_RHI_API  EndFrame();
        void VT_RENDERER_RHI_API  BeginRenderPass(Ref<Framebuffer> target);
        void VT_RENDERER_RHI_API  EndRenderPass();

        void VT_RENDERER_RHI_API  WaitIdle();

        void VT_RENDERER_RHI_API  Draw(Ref<GraphicsPipeline> pipeline,
                                       Ref<VertexBuffer>     vertexBuffer,
                                       Ref<IndexBuffer>      indexBuffer,
                                       u32 indexCount, Ref<Material> material);
        usize VT_RENDERER_RHI_API MemoryUsage();
        usize VT_RENDERER_RHI_API MemoryBudget();
    }; // namespace RHI
}; // namespace Vortex
