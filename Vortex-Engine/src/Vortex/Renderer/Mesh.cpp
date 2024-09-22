/*
 * Created by v1tr10l7 on 21.09.2024.
 * Copyright (c) 2024-2024, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#include "vtpch.hpp"

#include "Vortex/Renderer/Mesh.hpp"
#include "Vortex/Renderer/VertexBufferLayout.hpp"

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>

namespace Vortex
{
    Mesh::Mesh(PathView path)
    {

        Assimp::Importer importer;

        u32              flags = aiProcess_Triangulate | aiProcess_FlipUVs
                  | aiProcess_OptimizeMeshes | aiProcess_CalcTangentSpace
                  | aiProcess_SortByPType | aiProcess_GenNormals
                  | aiProcess_GenUVCoords | aiProcess_ValidateDataStructure;
        const aiScene* scene = importer.ReadFile(path, flags);
        VtCoreAssert(scene && scene->HasMeshes());

        ProcessNode(scene->mRootNode, scene);
        importer.FreeScene();

        m_VertexBuffer
            = VertexBuffer::Create((void*)m_Vertices.data(),
                                   m_Vertices.size() * sizeof(m_Vertices[0]));
        m_IndexBuffer = IndexBuffer::Create(m_Indices.data(),
                                            m_Indices.size() * sizeof(u32));
    }

    void Mesh::ProcessNode(aiNode* node, const aiScene* scene)
    {
        for (usize i = 0; i < node->mNumMeshes; i++)
        {
            aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
            ProcessMesh(mesh);
        }
        for (usize i = 0; i < node->mNumChildren; i++)
            ProcessNode(node->mChildren[i], scene);
    }
    void Mesh::ProcessMesh(aiMesh* mesh)
    {

        for (usize i = 0; i < mesh->mNumVertices; i++)
        {
            Vertex vertex{};
            vertex.Pos.x = mesh->mVertices[i].x;
            vertex.Pos.y = mesh->mVertices[i].y;
            vertex.Pos.z = mesh->mVertices[i].z;
            if (mesh->mTextureCoords[0])
            {
                vertex.TexCoords.x = mesh->mTextureCoords[0][i].x;
                vertex.TexCoords.y = mesh->mTextureCoords[0][i].y;
            }
            vertex.Color = Vec3(1.0f, 1.0f, 1.0f);
            m_Vertices.push_back(vertex);
        }

        for (u32 i = 0; i < mesh->mNumFaces; i++)
        {
            aiFace face = mesh->mFaces[i];
            for (u32 j = 0; j < face.mNumIndices; j++)
                m_Indices.push_back(face.mIndices[j]);
        }
    }
}; // namespace Vortex
