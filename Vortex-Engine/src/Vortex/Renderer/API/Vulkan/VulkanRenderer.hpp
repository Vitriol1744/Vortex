/*
 * Created by v1tr10l7 on 13.08.2024.
 * Copyright (c) 2024-2024, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include "Vortex/Renderer/API/RendererBackend.hpp"
#include "Vortex/Renderer/API/Vulkan/VulkanContext.hpp"
#include "Vortex/Renderer/API/Vulkan/VulkanGraphicsPipeline.hpp"

namespace Vortex
{
    class VulkanRenderer : public RendererBackend
    {
      public:
        virtual ~VulkanRenderer() = default;

        virtual void BeginFrame(Ref<Window> window) override;
        virtual void EndFrame() override;
        virtual void BeginRenderPass() override;
        virtual void EndRenderPass() override;

        virtual void Draw(Ref<GraphicsPipeline> pipeline,
                          Ref<VertexBuffer>     vertexBuffer,
                          Ref<IndexBuffer>      indexBuffer,
                          u32                   indexCount) override;

      private:
        Ref<VulkanContext> currentContext = nullptr;
    };
}; // namespace Vortex
