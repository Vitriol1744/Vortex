/*
 * Created by v1tr10l7 on 21.09.2024.
 * Copyright (c) 2024-2026, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#include <Vortex/Renderer/Mesh.hpp>
#include <Vortex/Renderer/VertexBufferLayout.hpp>

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>

namespace Vortex
{
    inline glm::mat4 AssimpToMat4(const aiMatrix4x4& from)
    {
        glm::mat4 to;
        to[0][0] = from.a1; to[1][0] = from.a2; to[2][0] = from.a3; to[3][0] = from.a4;
        to[0][1] = from.b1; to[1][1] = from.b2; to[2][1] = from.b3; to[3][1] = from.b4;
        to[0][2] = from.c1; to[1][2] = from.c2; to[2][2] = from.c3; to[3][2] = from.c4;
        to[0][3] = from.d1; to[1][3] = from.d2; to[2][3] = from.d3; to[3][3] = from.d4;
        return to;
    }

    Mesh::Mesh(PathView path)
        : m_FilePath(path)
    {
        Assimp::Importer importer;

        u32 flags = aiProcess_Triangulate | aiProcess_FlipUVs
                  | aiProcess_OptimizeMeshes | aiProcess_CalcTangentSpace
                  | aiProcess_SortByPType | aiProcess_GenNormals
                  | aiProcess_GenUVCoords | aiProcess_ValidateDataStructure;

        const aiScene* scene = importer.ReadFile(path.Raw(), flags);
        VtCoreAssert(scene && scene->HasMeshes());

        m_IsAnimated = scene->mAnimations != nullptr;
        VtCoreTrace("Mesh {} is animated: {}", path, m_IsAnimated);

        m_AABB.Min = {FLT_MAX, FLT_MAX, FLT_MAX};
        m_AABB.Max = {-FLT_MAX, -FLT_MAX, -FLT_MAX};

        ProcessNode(scene->mRootNode, scene, glm::mat4(1.0f));
        importer.FreeScene();

        m_VertexBuffer
            = VertexBuffer::Create(reinterpret_cast<void*>(m_Vertices.Raw()),
                                   m_Vertices.Size() * sizeof(m_Vertices[0]));
        m_IndexBuffer = IndexBuffer::Create(m_Indices.Raw(),
                                            m_Indices.Size() * sizeof(u32));
    }

    void Mesh::ProcessNode(aiNode* node, const aiScene* scene,
                           const glm::mat4& parentTransform)
    {
        glm::mat4 nodeTransform    = AssimpToMat4(node->mTransformation);
        glm::mat4 currentTransform = parentTransform * nodeTransform;

        u32 vertexCount = m_Vertices.Size();
        u32 indexCount  = m_Indices.Size();

        for (usize i = 0; i < node->mNumMeshes; i++)
        {
            aiMesh*  mesh         = scene->mMeshes[node->mMeshes[i]];
            Submesh& submesh      = m_Submeshes.EmplaceBack();
            submesh.BaseVertex    = vertexCount;
            submesh.BaseIndex     = indexCount;
            submesh.MaterialIndex = mesh->mMaterialIndex;
            submesh.VertexCount   = mesh->mNumVertices;
            submesh.IndexCount    = mesh->mNumFaces * 3;
            submesh.MeshName      = mesh->mName.C_Str();
            submesh.Transform     = currentTransform;

            ProcessMesh(mesh, submesh);

            vertexCount += submesh.VertexCount;
            indexCount += submesh.IndexCount;

            // Expand composite AABB using node-transformed submesh corners
            glm::vec3 corners[8] = {
                {submesh.BoundingBox.Min.x, submesh.BoundingBox.Min.y, submesh.BoundingBox.Min.z},
                {submesh.BoundingBox.Max.x, submesh.BoundingBox.Min.y, submesh.BoundingBox.Min.z},
                {submesh.BoundingBox.Min.x, submesh.BoundingBox.Max.y, submesh.BoundingBox.Min.z},
                {submesh.BoundingBox.Max.x, submesh.BoundingBox.Max.y, submesh.BoundingBox.Min.z},
                {submesh.BoundingBox.Min.x, submesh.BoundingBox.Min.y, submesh.BoundingBox.Max.z},
                {submesh.BoundingBox.Max.x, submesh.BoundingBox.Min.y, submesh.BoundingBox.Max.z},
                {submesh.BoundingBox.Min.x, submesh.BoundingBox.Max.y, submesh.BoundingBox.Max.z},
                {submesh.BoundingBox.Max.x, submesh.BoundingBox.Max.y, submesh.BoundingBox.Max.z}
            };

            for (const auto& corner : corners)
            {
                glm::vec3 transformed = glm::vec3(currentTransform * glm::vec4(corner, 1.0f));
                m_AABB.Min.x = Min(transformed.x, m_AABB.Min.x);
                m_AABB.Min.y = Min(transformed.y, m_AABB.Min.y);
                m_AABB.Min.z = Min(transformed.z, m_AABB.Min.z);
                m_AABB.Max.x = Max(transformed.x, m_AABB.Max.x);
                m_AABB.Max.y = Max(transformed.y, m_AABB.Max.y);
                m_AABB.Max.z = Max(transformed.z, m_AABB.Max.z);
            }
        }

        for (usize j = 0; j < node->mNumChildren; j++)
            ProcessNode(node->mChildren[j], scene, currentTransform);
    }

    void Mesh::ProcessMesh(aiMesh* mesh, Submesh& submesh)
    {
        auto& aabb = submesh.BoundingBox;
        aabb.Min   = {FLT_MAX, FLT_MAX, FLT_MAX};
        aabb.Max   = {-FLT_MAX, -FLT_MAX, -FLT_MAX};

        for (usize i = 0; i < mesh->mNumVertices; i++)
        {
            Vertex vertex{};
            vertex.Pos.x = mesh->mVertices[i].x;
            vertex.Pos.y = mesh->mVertices[i].y;
            vertex.Pos.z = mesh->mVertices[i].z;

            if (mesh->HasTextureCoords(0))
            {
                vertex.TexCoords.x = mesh->mTextureCoords[0][i].x;
                vertex.TexCoords.y = mesh->mTextureCoords[0][i].y;
            }

            vertex.Color = glm::vec3(1.0f, 1.0f, 1.0f);
            m_Vertices.PushBack(vertex);

            aabb.Min.x = Min(vertex.Pos.x, aabb.Min.x);
            aabb.Min.y = Min(vertex.Pos.y, aabb.Min.y);
            aabb.Min.z = Min(vertex.Pos.z, aabb.Min.z);
            aabb.Max.x = Max(vertex.Pos.x, aabb.Max.x);
            aabb.Max.y = Max(vertex.Pos.y, aabb.Max.y);
            aabb.Max.z = Max(vertex.Pos.z, aabb.Max.z);
        }

        // Add submesh.BaseVertex so indices index into global m_Vertices buffer correctly
        for (u32 i = 0; i < mesh->mNumFaces; i++)
        {
            aiFace face = mesh->mFaces[i];
            for (u32 j = 0; j < face.mNumIndices; j++)
                m_Indices.PushBack(submesh.BaseVertex + face.mIndices[j]);
        }
    }
}; // namespace Vortex
