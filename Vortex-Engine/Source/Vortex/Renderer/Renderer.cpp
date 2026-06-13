/*
 * Created by v1tr10l7 on 13.08.2024.
 * Copyright (c) 2024-2024, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#include <Vortex/Renderer/API/Vulkan/VulkanRenderer.hpp>
#include <Vortex/Renderer/Renderer.hpp>

namespace Vortex
{
    namespace Renderer
    {
        static RendererBackend*     s_Backend = nullptr;
        static struct Configuration s_Configuration;

        void                        Initialize()
        {
            s_Backend = new VulkanRenderer();
            s_Backend->Initialize();
        }
        void Shutdown()
        {
            s_Backend->Shutdown();
            delete s_Backend;
            s_Backend = nullptr;
        }

        const struct Configuration& Configuration() { return s_Configuration; }
        u32  CurrentFrameIndex() { return s_Backend->CurrentFrameIndex(); }

        void BeginFrame(Window& window) { s_Backend->BeginFrame(window); }
        void EndFrame() { s_Backend->EndFrame(); }
        void BeginRenderPass(Ref<Framebuffer> target)
        {
            s_Backend->BeginRenderPass(target);
        }
        void EndRenderPass() { s_Backend->EndRenderPass(); }

        void Draw(Ref<GraphicsPipeline> pipeline,
                  Ref<VertexBuffer> vertexBuffer, Ref<IndexBuffer> indexBuffer,
                  u32 indexCount, Ref<Material> material)
        {
            s_Backend->Draw(pipeline, vertexBuffer, indexBuffer, indexCount,
                            material);
        }

        usize MemoryUsage() { return s_Backend->MemoryUsage(); }
        usize MemoryBudget() { return s_Backend->MemoryBudget(); }
    }; // namespace Renderer
}; // namespace Vortex
