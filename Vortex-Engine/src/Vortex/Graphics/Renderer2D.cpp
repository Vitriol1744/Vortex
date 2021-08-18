//
// Created by vitriol1744 on 19.07.2021.
//
#include "Renderer2D.hpp"

#include "Vortex/Graphics/API/IShader.hpp"
#include "Vortex/Graphics/API/IVertexArray.hpp"
#include "Vortex/Graphics/API/IRendererAPI.hpp"

using namespace Vortex::Math;

namespace Vortex::Graphics
{
    namespace
    {
        struct Vertex
        {
            Vec3 position;
            Vec4 color;
            Vec2 texCoords;
            float32 textureIndex;
        };

        constexpr uint32 maxSpritesCount    = 1000;
        constexpr uint32 maxVerticesCount   = maxSpritesCount * 4;
        constexpr uint32 maxIndicesCount    = maxSpritesCount * 6;

        Vec4 defaultPositions[] =
        {
            { -0.5f,  0.5f, 0.0f, 1.0f  },
            {  0.5f,  0.5f, 0.0f, 1.0f  },
            { -0.5f, -0.5f, 0.0f, 1.0f  },
            {  0.5f, -0.5f, 0.0f, 1.0f  }
        };

        Ref<IShader> shader = nullptr;
        Ref<IVertexArray> mesh = nullptr;
        Ref<IVertexBuffer> vertexBuffer = nullptr;
        Ref<IIndexBuffer> indexBuffer = nullptr;

        Vertex* vertices = nullptr;
        Vertex* current = nullptr;
        uint32* indices = nullptr;
        uint32 indicesCount = 0;
        bool initialized = false;

        Mat4 viewProjectionMatrix;
    }

    void Renderer2D::BeginScene(const Camera& camera)
    {
        viewProjectionMatrix = const_cast<Camera&>(camera).GetViewProjectionMatrix();
    }
    void Renderer2D::EndScene()
    {
        viewProjectionMatrix = glm::mat4(1.0f);
    }

    void Renderer2D::Initialize()
    {
        if (initialized) return;
        mesh = IVertexArray::Create();
        shader = IShader::Create("assets/shaders/batch.vert", "assets/shaders/batch.frag", false);
        vertexBuffer = IVertexBuffer::Create(maxVerticesCount * sizeof(Vertex));

        if (!vertices)
        {
            vertices = new Vertex[maxVerticesCount];
            current = vertices;
        }
        if (!indices) indices = new uint32[maxIndicesCount];

        VertexBufferLayout layout;
        layout.AddElement(ShaderDataType::Float3); // Position
        layout.AddElement(ShaderDataType::Float4); // Color
        layout.AddElement(ShaderDataType::Float2); // Texture Coords
        layout.AddElement(ShaderDataType::Float);  // Texture Index
        vertexBuffer->SetLayout(layout);
        mesh->AddVertexBuffer(vertexBuffer);

        uint32 offset = 0;
        for (uint32 i = 0; i < maxIndicesCount; i += 6)
        {
            indices[i + 0] = offset + 0;
            indices[i + 1] = offset + 1;
            indices[i + 2] = offset + 2;

            indices[i + 3] = offset + 1;
            indices[i + 4] = offset + 3;
            indices[i + 5] = offset + 2;

            offset += 4;
        }

        indexBuffer = IIndexBuffer::Create(indices, maxIndicesCount);
        mesh->SetIndexBuffer(indexBuffer);
        delete[] indices;

        initialized = true;
    }
    void Renderer2D::Shutdown()
    {
        if (!initialized) return;
        if (vertices)
        {
            delete[] vertices;
            vertices = nullptr;
        }
        initialized = false;
    }

    void Renderer2D::DrawSprite(Sprite sprite)
    {
        if (indicesCount >= maxIndicesCount)
        {
            Submit();
        }

        Vec2 textureCoords[] = { { 0.0f, 1.0f }, { 1.0f, 1.0f }, { 0.0f, 0.0f }, { 1.0f, 0.0f } };

        for (int32 i = 0; i < 4; i++)
        {
            current->position = sprite.transform * defaultPositions[i];
            current->color = sprite.color;
            current->texCoords = textureCoords[i];
            current->textureIndex = 0;
            current++;
        }

        indicesCount += 6;
    }
    void Renderer2D::DrawQuad(const Vec2& position, const Vec2& size, const Vec4& color)
    {
        if (indicesCount >= maxIndicesCount) Submit();

        Vec2 textureCoords[] = { { 0.0f, 1.0f }, { 1.0f, 1.0f }, { 0.0f, 0.0f }, { 1.0f, 0.0f } };
    
        for (int32 i = 0; i < 4; i++)
        {
            current->position = glm::vec4(position, 0, 1) * defaultPositions[i];
            current->color = color;
            current->texCoords = textureCoords[i];
            current->textureIndex = 0;
            current++;
        }
    
        indicesCount += 6;
    }
    
    void Renderer2D::Submit()
    {
        mesh->Bind();
        shader->Bind();
        
        shader->SetUniformMat4f("u_ViewProjection", viewProjectionMatrix);
        uint32 dataSize = (uint32)((uint8*)current - (uint8*)vertices);
        vertexBuffer->SetData(vertices, dataSize);

        IRendererAPI::DrawIndexed(mesh, indicesCount);

        indicesCount = 0;
        current = vertices;
    }
}
