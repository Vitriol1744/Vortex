/*
 * Created by v1tr10l7 on 13.08.2024.
 * Copyright (c) 2024-2024, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include <Vortex/Renderer/RHI/RHI.hpp>
#include <Vortex/Renderer/ShaderLibrary.hpp>

namespace Vortex
{
    namespace Renderer
    {
        void Initialize(const RHI::Specification& specification);
        void Shutdown();

        class ShaderLibrary& ShaderLibrary();

        // FIXME(v1tr10l7): those resource creation functions should be
        // somewhere else
        ::Ref<Framebuffer>
        CreateFramebuffer(const FramebufferSpecification& spec);
        ::Ref<GraphicsPipeline>
        CreateGraphicsPipeline(const GraphicsPipelineSpecification& spec);
        ::Ref<VertexBuffer>  CreateVertexBuffer(void* data, usize size);
        ::Ref<IndexBuffer>   CreateIndexBuffer(u32* data, usize size);
        ::Ref<UniformBuffer> CreateUniformBuffer(usize size);
        ::Ref<SwapChain>     CreateSwapChain(class Window* window,
                                             bool          vsync = false);
        ::Ref<Shader>        CreateShader(PathView path);
        ::Ref<Shader> CreateShader(PathView vertexPath, PathView fragmentPath);
        ::Ref<Material>  CreateMaterial(const ::Ref<Shader> shader);
        ::Ref<Texture2D> CreateTexture2D(ImageView image);

        u32              CurrentFrameIndex();

        void             BeginFrame(Ref<SwapChain> swapChain);
        void             EndFrame();
        void             BeginRenderPass(Ref<Framebuffer> target);
        void             EndRenderPass();

        void             WaitIdle();

        void             Draw(Ref<GraphicsPipeline> pipeline,
                              Ref<VertexBuffer> vertexBuffer, Ref<IndexBuffer> indexBuffer,
                              u32 indexCount, Ref<Material> material = nullptr);

        inline void      Draw(Ref<GraphicsPipeline> pipeline,
                              Ref<VertexBuffer>     vertexBuffer,
                              Ref<IndexBuffer>      indexBuffer,
                              Ref<Material>         material = nullptr)
        {
            Draw(pipeline, vertexBuffer, indexBuffer, indexBuffer->Count(),
                 material);
        }

        usize MemoryUsage();
        usize MemoryBudget();
    }; // namespace Renderer
}; // namespace Vortex
