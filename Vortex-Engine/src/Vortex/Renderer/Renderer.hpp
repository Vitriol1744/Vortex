/*
 * Created by v1tr10l7 on 13.08.2024.
 * Copyright (c) 2024-2024, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include "Vortex/Renderer/API/Framebuffer.hpp"
#include "Vortex/Renderer/API/GraphicsPipeline.hpp"
#include "Vortex/Renderer/API/IndexBuffer.hpp"
#include "Vortex/Renderer/API/VertexBuffer.hpp"

namespace Vortex
{
    namespace Renderer
    {
        struct Configuration
        {
            u32 MaxFramesInFlight = 2;
        };

        void                 Initialize();
        void                 Shutdown();

        const Configuration& GetConfiguration();
        u32                  GetCurrentFrameIndex();

        void                 BeginFrame(Window& window);
        void                 EndFrame();
        void                 BeginRenderPass(Ref<Framebuffer> target);
        void                 EndRenderPass();

        void                 Draw(Ref<GraphicsPipeline> pipeline,
                                  Ref<VertexBuffer> vertexBuffer, Ref<IndexBuffer> indexBuffer,
                                  u32 indexCount);

        inline void          Draw(Ref<GraphicsPipeline> pipeline,
                                  Ref<VertexBuffer>     vertexBuffer,
                                  Ref<IndexBuffer>      indexBuffer)
        {
            Draw(pipeline, vertexBuffer, indexBuffer, indexBuffer->GetCount());
        }

        usize GetMemoryUsage();
        usize GetMemoryBudget();
    }; // namespace Renderer
};     // namespace Vortex
