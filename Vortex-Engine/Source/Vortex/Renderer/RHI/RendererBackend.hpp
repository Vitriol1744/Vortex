/*
 * Created by v1tr10l7 on 13.08.2024.
 * Copyright (c) 2024-2024, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include <Vortex/Core/Core.hpp>
#include <Vortex/Renderer/RHI/Framebuffer.hpp>
#include <Vortex/Renderer/RHI/GraphicsPipeline.hpp>
#include <Vortex/Renderer/RHI/IndexBuffer.hpp>
#include <Vortex/Renderer/RHI/Material.hpp>
#include <Vortex/Renderer/RHI/RHI.hpp>
#include <Vortex/Renderer/RHI/SwapChain.hpp>
#include <Vortex/Renderer/RHI/VertexBuffer.hpp>

namespace Vortex
{
    class VT_RENDERER_RHI_API RendererBackend
        : public NonCopyable<RendererBackend>,
          public RefCounted<RendererBackend>
    {
      public:
        virtual ~RendererBackend()                               = default;

        virtual u32  CurrentFrameIndex()                         = 0;

        virtual void Initialize(const RHI::Specification& specs) = 0;
        virtual void Shutdown()                                  = 0;

        virtual ::Ref<Framebuffer>
        CreateFramebuffer(const FramebufferSpecification& spec) = 0;
        virtual ::Ref<GraphicsPipeline>
        CreateGraphicsPipeline(const GraphicsPipelineSpecification& spec) = 0;
        virtual ::Ref<VertexBuffer> CreateVertexBuffer(void* data, usize size)
            = 0;
        virtual ::Ref<IndexBuffer> CreateIndexBuffer(u32* data, usize size) = 0;
        virtual ::Ref<UniformBuffer> CreateUniformBuffer(usize size)        = 0;
        virtual ::Ref<SwapChain>     CreateSwapChain(class Window* window,
                                                     bool          vsync = false)
            = 0;
        virtual ::Ref<Shader> CreateShader(PathView path) = 0;
        virtual ::Ref<Shader> CreateShader(PathView vertexPath,
                                           PathView fragmentPath)
            = 0;
        virtual ::Ref<Material>  CreateMaterial(const ::Ref<Shader> shader) = 0;
        virtual ::Ref<Texture2D> CreateTexture2D(ImageView image)           = 0;

        virtual void             BeginFrame(::Ref<SwapChain> swapChain)     = 0;
        virtual void             EndFrame()                                 = 0;
        // TODO(v1tr10l7): implement custom render passes
        virtual void             BeginRenderPass(::Ref<Framebuffer> target) = 0;
        virtual void             EndRenderPass()                            = 0;

        virtual void             WaitIdle()                                 = 0;

        virtual void             Draw(::Ref<GraphicsPipeline> pipeline,
                                      ::Ref<VertexBuffer>     vertexBuffer,
                                      ::Ref<IndexBuffer> indexBuffer, u32 indexCount,
                                      ::Ref<Material> material = nullptr)
            = 0;

        virtual usize MemoryUsage()  = 0;
        virtual usize MemoryBudget() = 0;
    };
}; // namespace Vortex
