/*
 * Created by v1tr10l7 on 13.08.2024.
 * Copyright (c) 2024-2024, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#include <Vortex/Renderer/RHI/RHI.hpp>
#include <Vortex/Renderer/RHI/Texture2D.hpp>
#include <Vortex/Renderer/RHI/UniformBuffer.hpp>
#include <Vortex/Renderer/RHI/Vulkan/VulkanRenderer.hpp>
#include <Vortex/Renderer/Renderer.hpp>

namespace Vortex
{
    namespace Renderer
    {
        static class ShaderLibrary s_ShaderLibrary;

        void                       Initialize(const RHI::Specification& specs)
        {
            VtCoreAssert(RHI::Initialize(specs));
        }
        void                 Shutdown() { RHI::Shutdown(); }

        class ShaderLibrary& ShaderLibrary() { return s_ShaderLibrary; }

        ::Ref<Framebuffer>
        CreateFramebuffer(const FramebufferSpecification& spec)
        {
            return RHI::CreateFramebuffer(spec);
        }
        ::Ref<GraphicsPipeline>
        CreateGraphicsPipeline(const GraphicsPipelineSpecification& spec)
        {
            return RHI::CreateGraphicsPipeline(spec);
        }
        ::Ref<VertexBuffer> CreateVertexBuffer(void* data, usize size)
        {
            return RHI::CreateVertexBuffer(data, size);
        }
        ::Ref<IndexBuffer> CreateIndexBuffer(u32* data, usize size)
        {
            return RHI::CreateIndexBuffer(data, size);
        }
        ::Ref<UniformBuffer> CreateUniformBuffer(usize size)
        {
            return RHI::CreateUniformBuffer(size);
        }
        ::Ref<SwapChain> CreateSwapChain(class Window* window, bool vsync)
        {
            return RHI::CreateSwapChain(window, vsync);
        }
        ::Ref<Shader> CreateShader(PathView path)
        {
            return RHI::CreateShader(path);
        }
        ::Ref<Shader> CreateShader(PathView vertexPath, PathView fragmentPath)
        {
            return RHI::CreateShader(vertexPath, fragmentPath);
        }
        ::Ref<Material> CreateMaterial(const ::Ref<Shader> shader)
        {
            return RHI::CreateMaterial(shader);
        }
        ::Ref<Texture2D> CreateTexture2D(ImageView image)
        {
            return RHI::CreateTexture2D(image);
        }

        u32  CurrentFrameIndex() { return RHI::CurrentFrameIndex(); }

        void BeginFrame(Ref<SwapChain> swapChain)
        {
            return RHI::BeginFrame(swapChain);
        }
        void EndFrame() { RHI::EndFrame(); }
        void BeginRenderPass(Ref<Framebuffer> target)
        {
            RHI::BeginRenderPass(target);
        }
        void EndRenderPass() { RHI::EndRenderPass(); }

        void WaitIdle() { RHI::WaitIdle(); }

        void Draw(Ref<GraphicsPipeline> pipeline,
                  Ref<VertexBuffer> vertexBuffer, Ref<IndexBuffer> indexBuffer,
                  u32 indexCount, Ref<Material> material)
        {
            RHI::Draw(pipeline, vertexBuffer, indexBuffer, indexCount,
                      material);
        }

        usize MemoryUsage() { return RHI::MemoryUsage(); }
        usize MemoryBudget() { return RHI::MemoryBudget(); }
    }; // namespace Renderer
}; // namespace Vortex
