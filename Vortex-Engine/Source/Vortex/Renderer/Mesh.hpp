/*
 * Created by v1tr10l7 on 21.09.2024.
 * Copyright (c) 2024-2024, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include <Prism/Utility/Path.hpp>
#include <Vortex/Core/Math/AABB.hpp>

#include <Vortex/Renderer/API/IndexBuffer.hpp>
#include <Vortex/Renderer/API/VertexBuffer.hpp>

#include <assimp/scene.h>

namespace Vortex
{
    class Submesh
    {
      public:
        u32        BaseVertex;
        u32        BaseIndex;
        u32        MaterialIndex;
        u32        IndexCount;
        u32        VertexCount;

        glm::mat4  Transform{1.0f};
        Math::AABB BoundingBox;

        String     NodeName;
        String     MeshName;
    };

    class VT_API Mesh : public RefCounted
    {
      public:
        explicit Mesh(PathView path);

        inline constexpr PathView  FilePath() const { return m_FilePath; }

        inline ::Ref<VertexBuffer> VertexBuffer() const
        {
            return m_VertexBuffer;
        }
        inline ::Ref<IndexBuffer> IndexBuffer() const { return m_IndexBuffer; }

      private:
        struct Vertex
        {
            Math::Vec3 Pos;
            Math::Vec3 Color;
            Math::Vec2 TexCoords;
        };

        Path                      m_FilePath;
        bool                      m_IsAnimated = false;
        Vector<Vertex>            m_Vertices;
        Vector<u32>               m_Indices;
        Vector<Submesh>           m_Submeshes;
        ::Ref<class VertexBuffer> m_VertexBuffer = nullptr;
        ::Ref<class IndexBuffer>  m_IndexBuffer  = nullptr;

        void ProcessNode(aiNode* node, const aiScene* scene);
        void ProcessMesh(aiMesh* mesh);
    };
}; // namespace Vortex
