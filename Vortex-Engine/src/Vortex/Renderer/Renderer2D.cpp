/*
 * Created by v1tr10l7 on 24.08.2024.
 * Copyright (c) 2024-2024, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#include "vtpch.hpp"

#include "Vortex/Core/Math/Math.hpp"

#include "Vortex/Engine/Application.hpp"
#include "Vortex/Renderer/API/GraphicsPipeline.hpp"
#include "Vortex/Renderer/API/IndexBuffer.hpp"
#include "Vortex/Renderer/API/Shader.hpp"
#include "Vortex/Renderer/API/Texture2D.hpp"
#include "Vortex/Renderer/API/UniformBuffer.hpp"
#include "Vortex/Renderer/API/VertexBuffer.hpp"
#include "Vortex/Renderer/API/Vulkan/VulkanContext.hpp"
#include "Vortex/Renderer/Renderer.hpp"
#include "Vortex/Renderer/Renderer2D.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace Vortex::Renderer2D
{
    namespace
    {
        struct Vertex
        {
            Vec3 Position;
            Vec3 Color;
            Vec2 TexCoords;
        };

        struct UniformBufferObject
        {
            alignas(16) Mat4 Model;
            alignas(16) Mat4 View;
            alignas(16) Mat4 Projection;
            alignas(8) Vec2 lightPos;
        };

        inline constexpr u32      MAX_QUADS             = 20000;
        inline constexpr u32      MAX_VERTICES          = MAX_QUADS * 4;
        inline constexpr u32      MAX_INDICES           = MAX_QUADS * 6;

        Ref<GraphicsPipeline>     s_QuadPipeline        = nullptr;
        Ref<VertexBuffer>         s_QuadVertexBuffer    = nullptr;
        Vertex*                   s_QuadVerticesBase    = nullptr;
        Vertex*                   s_QuadVerticesPointer = nullptr;
        Ref<IndexBuffer>          s_QuadIndexBuffer     = nullptr;
        Ref<Shader>               s_TextureShader       = nullptr;
        Ref<Texture2D>            s_WhiteTexture        = nullptr;
        u32                       s_IndexCount          = 0;
        static Ref<UniformBuffer> s_UniformBuffer       = nullptr;
    }; // namespace

    void Initialize()
    {
        std::initializer_list<VertexBufferElement> elements
            = {ShaderDataType::eFloat3, ShaderDataType::eFloat3,
               ShaderDataType::eFloat2};

        s_TextureShader = Shader::Create("assets/shaders/texture.glsl");
        VertexBufferLayout            layout(elements);
        GraphicsPipelineSpecification specification{};
        specification.Window  = Application::Get()->GetWindow();
        specification.Shader  = s_TextureShader;
        specification.Layout  = layout;

        s_QuadPipeline        = GraphicsPipeline::Create(specification);
        s_QuadVerticesBase    = new Vertex[MAX_VERTICES];
        s_QuadVerticesPointer = s_QuadVerticesBase;

        u32* quadIndices      = new u32[MAX_INDICES];

        for (u32 i = 0, offset = 0; i < MAX_INDICES; i += 6, offset += 4)
        {
            quadIndices[i + 0] = offset + 0;
            quadIndices[i + 1] = offset + 1;
            quadIndices[i + 2] = offset + 2;

            quadIndices[i + 3] = offset + 2;
            quadIndices[i + 4] = offset + 3;
            quadIndices[i + 5] = offset + 0;
        }

        s_QuadIndexBuffer
            = IndexBuffer::Create(quadIndices, MAX_INDICES * sizeof(u32));
        delete[] quadIndices;

        s_WhiteTexture  = Texture2D::Create("assets/textures/texture.jpg");
        s_UniformBuffer = UniformBuffer::Create(sizeof(UniformBufferObject));

        s_TextureShader->SetUniform("UniformBufferObject", s_UniformBuffer);
        s_TextureShader->SetUniform("texSampler", s_WhiteTexture);
    }
    void Shutdown() { delete[] s_QuadVerticesBase; }

    void DrawQuad(const Vec2& pos, const Vec2& scale, const Vec4& color)
    {
        constexpr size_t    quadVertexCount = 4;
        constexpr glm::vec2 textureCoords[]
            = {{1.0f, 0.0f}, {0.0f, 0.0f}, {0.0f, 1.0f}, {1.0f, 1.0f}};

        if (s_IndexCount >= MAX_INDICES) Flush();

        static Vec4 quadVertexPositions[4];
        quadVertexPositions[0] = {-0.5f, -0.5f, 0.0f, 1.0f};
        quadVertexPositions[1] = {0.5f, -0.5f, 0.0f, 1.0f};
        quadVertexPositions[2] = {0.5f, 0.5f, 0.0f, 1.0f};
        quadVertexPositions[3] = {-0.5f, 0.5f, 0.0f, 1.0f};

        (void)scale;
        glm::mat4 transform
            = glm::translate(glm::mat4(1.0f), glm::vec3(pos, 0.0f))
            * glm::scale(glm::mat4(1.0f), glm::vec3(scale, 1.0f));
        for (usize i = 0; i < quadVertexCount; i++)
        {
            s_QuadVerticesPointer->Position
                = transform * quadVertexPositions[i];
            s_QuadVerticesPointer->Color     = color;
            s_QuadVerticesPointer->TexCoords = textureCoords[i];
            s_QuadVerticesPointer++;
        }

        s_IndexCount += 6;
    }
    void Flush()
    {
        u32 dataSize
            = u32((u8*)s_QuadVerticesPointer - (u8*)s_QuadVerticesBase);
        s_QuadVertexBuffer = VertexBuffer::Create(s_QuadVerticesBase, dataSize);

        auto window        = Application::Get()->GetWindow();
        Vec2 mousePos      = window->GetCursorPosition();
        mousePos.x -= window->GetWidth() / 2.f;
        mousePos.y -= window->GetHeight() / 2.f;
        mousePos.x /= window->GetWidth();
        mousePos.y /= window->GetHeight();
        mousePos.x *= -1;

        UniformBufferObject ubo{};
        ubo.lightPos = mousePos;
        ubo.Model    = glm::mat4(1.0f);
        ubo.View = glm::lookAt(Vec3(2.0f, 2.0f, 2.0f), Vec3(0.0f, 0.0f, 0.0f),
                               Vec3(0.0f, 0.0f, 1.0f));
        // TODO(v1tr10l7): do it on begin scene
        f32 aspectRatio = f32(window->GetWidth()) / f32(window->GetHeight());
        ubo.Projection
            = glm::perspective(glm::radians(45.0f), aspectRatio, 0.1f, 10.0f);
        ubo.Projection[1][1] *= -1;
        s_UniformBuffer->SetData(&ubo, sizeof(ubo), 0);

        Renderer::Draw(s_QuadPipeline, s_QuadVertexBuffer, s_QuadIndexBuffer,
                       s_IndexCount);
        s_QuadVerticesPointer = s_QuadVerticesBase;
        s_IndexCount          = 0;
    }
}; // namespace Vortex::Renderer2D
