/*
 * Created by v1tr10l7 on 21.09.2024.
 * Copyright (c) 2024-2024, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include "Vortex/Core/Math/Vector.hpp"

#include "Vortex/Renderer/API/IndexBuffer.hpp"
#include "Vortex/Renderer/API/VertexBuffer.hpp"

#include <assimp/scene.h>

namespace Vortex
{
    class VT_API Mesh
    {
      public:
        explicit Mesh(PathView path);

        inline Ref<VertexBuffer> GetVertexBuffer() const
        {
            return m_VertexBuffer;
        }
        inline Ref<IndexBuffer> GetIndexBuffer() const { return m_IndexBuffer; }

      private:
        struct Vertex
        {
            Vec3 Pos;
            Vec3 Color;
            Vec2 TexCoords;
        };

        std::vector<Vertex> m_Vertices;
        std::vector<u32>    m_Indices;
        Ref<VertexBuffer>   m_VertexBuffer = nullptr;
        Ref<IndexBuffer>    m_IndexBuffer  = nullptr;

        void                ProcessNode(aiNode* node, const aiScene* scene);
        void                ProcessMesh(aiMesh* mesh);
    };
}; // namespace Vortex
