/*
 * Created by v1tr10l7 on 10.07.2026.
 * Copyright (c) 2024-2026, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#include <Vortex/Renderer/Image.hpp>
#include <Vortex/Renderer/RHI/RHI.hpp>
#include <Vortex/Renderer/RHI/RendererBackend.hpp>
#include <Vortex/Renderer/RHI/Vulkan/VulkanRenderer.hpp>

namespace Vortex
{
    namespace RHI
    {
        static Scope<RendererBackend> s_RHI = nullptr;
        static Specification          s_Specification;

        bool VT_RENDERER_RHI_API      Initialize(const Specification& specs)
        {
            VtCoreAssert(!s_RHI && "RHI: Already initialized")

                switch (specs.API)
            {
                case API::eVulkan:
                    s_RHI = CreateScope<VulkanRenderer>();
                    s_RHI->Initialize(specs);
                    s_Specification = specs;
                    return true;

                default: VtCoreAssert("RHI: Unsupported API"); return false;
            }
        }
        void VT_RENDERER_RHI_API Shutdown()
        {
            if (!s_RHI) return;

            s_RHI->Shutdown();
            s_RHI.Reset();
        }

        const Specification& VT_RENDERER_RHI_API Configuration()
        {
            return s_Specification;
        }

        ::Ref<Framebuffer> VT_RENDERER_RHI_API
        CreateFramebuffer(const FramebufferSpecification& spec)
        {
            return s_RHI->CreateFramebuffer(spec);
        }
        ::Ref<GraphicsPipeline> VT_RENDERER_RHI_API
        CreateGraphicsPipeline(const GraphicsPipelineSpecification& spec)
        {
            return s_RHI->CreateGraphicsPipeline(spec);
        }
        ::Ref<VertexBuffer> VT_RENDERER_RHI_API CreateVertexBuffer(void* data,
                                                                   usize size)
        {
            return s_RHI->CreateVertexBuffer(data, size);
        }
        ::Ref<IndexBuffer> VT_RENDERER_RHI_API CreateIndexBuffer(u32*  data,
                                                                 usize size)
        {
            return s_RHI->CreateIndexBuffer(data, size);
        }
        ::Ref<UniformBuffer> VT_RENDERER_RHI_API CreateUniformBuffer(usize size)
        {
            return s_RHI->CreateUniformBuffer(size);
        }
        ::Ref<SwapChain> VT_RENDERER_RHI_API
        CreateSwapChain(class Window* window, bool vsync)
        {
            return s_RHI->CreateSwapChain(window, vsync);
        }
        ::Ref<Shader> VT_RENDERER_RHI_API CreateShader(PathView path)
        {
            return s_RHI->CreateShader(path);
        }
        ::Ref<Shader> VT_RENDERER_RHI_API CreateShader(PathView vertexPath,
                                                       PathView fragmentPath)
        {
            return s_RHI->CreateShader(vertexPath, fragmentPath);
        }
        ::Ref<Material>
            VT_RENDERER_RHI_API CreateMaterial(const ::Ref<Shader> shader)
        {
            return s_RHI->CreateMaterial(shader);
        }
        ::Ref<Texture2D> VT_RENDERER_RHI_API CreateTexture2D(ImageView image)
        {
            return s_RHI->CreateTexture2D(image);
        }

        u32 VT_RENDERER_RHI_API CurrentFrameIndex()
        {
            return s_RHI->CurrentFrameIndex();
        }

        void VT_RENDERER_RHI_API BeginFrame(Ref<SwapChain> swapChain)
        {
            s_RHI->BeginFrame(swapChain);
        }
        void VT_RENDERER_RHI_API EndFrame() { s_RHI->EndFrame(); }
        void VT_RENDERER_RHI_API BeginRenderPass(Ref<Framebuffer> target)
        {
            s_RHI->BeginRenderPass(target);
        }
        void VT_RENDERER_RHI_API EndRenderPass() { s_RHI->EndRenderPass(); }

        void VT_RENDERER_RHI_API WaitIdle() { s_RHI->WaitIdle(); }

        void VT_RENDERER_RHI_API Draw(Ref<GraphicsPipeline> pipeline,
                                      Ref<VertexBuffer>     vertexBuffer,
                                      Ref<IndexBuffer>      indexBuffer,
                                      u32 indexCount, Ref<Material> material)
        {
            s_RHI->Draw(pipeline, vertexBuffer, indexBuffer, indexCount,
                        material);
        }

        usize VT_RENDERER_RHI_API MemoryUsage() { return s_RHI->MemoryUsage(); }
        usize VT_RENDERER_RHI_API MemoryBudget()
        {
            return s_RHI->MemoryBudget();
        }
    }; // namespace RHI
}; // namespace Vortex
