/*
 * Created by v1tr10l7 on 24.05.2026.
 * Copyright (c) 2024-2026, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include <Vortex/Renderer/API/Framebuffer.hpp>
#include <Vortex/Renderer/API/GraphicsPipeline.hpp>
#include <Vortex/Renderer/API/Shader.hpp>

namespace Vortex
{
    namespace SceneRenderer
    {
        using PipelineSpecification = GraphicsPipelineSpecification;
        using Pipeline              = GraphicsPipeline;

        void             Initialize();
        void             Shutdown();

        void             BeginScene();
        void             EndScene();

        Ref<Pipeline>    GraphicsPipeline();
        Ref<Shader>      Shader();
        Ref<Framebuffer> Framebuffer();
    }; // namespace SceneRenderer
}; // namespace Vortex
