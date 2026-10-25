/*
 * Created by v1tr10l7 on 24.05.2026.
 * Copyright (c) 2024-2026, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#include <Vortex/Asset/AssetManager.hpp>
#include <Vortex/Renderer/Renderer.hpp>
#include <Vortex/Renderer/SceneRenderer.hpp>

namespace Vortex
{
    namespace SceneRenderer
    {
        namespace
        {
            Ref<Pipeline>          s_Pipeline;
            Ref<class Framebuffer> s_Framebuffer;
        } // namespace

        void Initialize(::Ref<class Framebuffer> framebuffer)
        {
            auto shader = Shader::Create("assets/shaders/texture.glsl");
            Renderer::ShaderLibrary().Add("TextureShader", shader);

            InitializerList<VertexBufferElement> elements
                = {ShaderDataType::eFloat3, ShaderDataType::eFloat3,
                   ShaderDataType::eFloat2};
            VertexBufferLayout    layout(elements);

            PipelineSpecification pipelineSpecs{};
            pipelineSpecs.Shader      = shader;
            pipelineSpecs.Framebuffer = framebuffer;
            pipelineSpecs.Layout = layout;
            s_Pipeline           = Pipeline::Create(pipelineSpecs);

            FramebufferSpecification framebufferSpecs{};
            framebufferSpecs.SwapChainTarget = false;
            framebufferSpecs.Width           = 1280;
            framebufferSpecs.Height          = 800;
            s_Framebuffer = Framebuffer::Create(framebufferSpecs);
        }
        void Shutdown()
        {
            s_Pipeline.Reset();
            s_Framebuffer.Reset();
        }

        void              BeginScene();
        void              EndScene();

        Ref<Pipeline>     GraphicsPipeline() { return s_Pipeline; }
        Ref<class Shader> Shader()
        {
            return Renderer::ShaderLibrary().Get("TextureShader");
        }
        Ref<class Framebuffer> Framebuffer() { return s_Framebuffer; }
    }; // namespace SceneRenderer
}; // namespace Vortex
