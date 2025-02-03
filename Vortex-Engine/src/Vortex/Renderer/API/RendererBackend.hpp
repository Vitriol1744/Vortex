/*
 * Created by v1tr10l7 on 13.08.2024.
 * Copyright (c) 2024-2024, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include "Vortex/Core/Core.hpp"
#include "Vortex/Renderer/API/Framebuffer.hpp"
#include "Vortex/Renderer/API/GraphicsPipeline.hpp"
#include "Vortex/Renderer/API/IndexBuffer.hpp"
#include "Vortex/Renderer/API/Material.hpp"
#include "Vortex/Renderer/API/VertexBuffer.hpp"
#include "Vortex/Window/Window.hpp"

namespace Vortex
{
    class VT_API RendererBackend
    {
      public:
        virtual ~RendererBackend()                            = default;

        virtual u32  GetCurrentFrameIndex()                   = 0;

        virtual void Initialize()                             = 0;
        virtual void Shutdown()                               = 0;

        virtual void BeginFrame(Window& window)               = 0;
        virtual void EndFrame()                               = 0;
        // TODO(v1tr10l7): implement custom render passes
        virtual void BeginRenderPass(Ref<Framebuffer> target) = 0;
        virtual void EndRenderPass()                          = 0;

        virtual void Draw(Ref<GraphicsPipeline> pipeline,
                          Ref<VertexBuffer>     vertexBuffer,
                          Ref<IndexBuffer> indexBuffer, u32 indexCount,
                          Ref<Material> material = nullptr)
            = 0;

        virtual usize GetMemoryUsage()  = 0;
        virtual usize GetMemoryBudget() = 0;
    };
}; // namespace Vortex
