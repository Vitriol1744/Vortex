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

        void         BeginFrame(Ref<Window> window) override;
        void         EndFrame() override;

        virtual void Draw(Ref<GraphicsPipeline> pipeline,
                          Ref<VertexBuffer>     vertexBuffer,
                          Ref<IndexBuffer>      indexBuffer) override;

      private:
        Ref<VulkanContext> currentContext = nullptr;
    };
}; // namespace Vortex
