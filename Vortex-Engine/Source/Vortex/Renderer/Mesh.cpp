/*
 * Created by v1tr10l7 on 21.09.2024.
 * Copyright (c) 2024-2024, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#include <Vortex/Renderer/Mesh.hpp>
#include <Vortex/Renderer/VertexBufferLayout.hpp>

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>

namespace Vortex
{
    Mesh::Mesh(PathView path)
        : m_FilePath(path)
    {

        Assimp::Importer importer;

        u32              flags = aiProcess_Triangulate | aiProcess_FlipUVs
                  | aiProcess_OptimizeMeshes | aiProcess_CalcTangentSpace
                  | aiProcess_SortByPType | aiProcess_GenNormals
                  | aiProcess_GenUVCoords | aiProcess_ValidateDataStructure;
        const aiScene* scene = importer.ReadFile(path.Raw(), flags);
        VtCoreAssert(scene && scene->HasMeshes());

        m_IsAnimated = scene->mAnimations != nullptr;
        VtCoreTrace("Mesh {} is animated: {}", path, m_IsAnimated);

        ProcessNode(scene->mRootNode, scene);
        importer.FreeScene();

        m_VertexBuffer
            = VertexBuffer::Create(reinterpret_cast<void*>(m_Vertices.Raw()),
                                   m_Vertices.Size() * sizeof(m_Vertices[0]));
        m_IndexBuffer = IndexBuffer::Create(m_Indices.Raw(),
                                            m_Indices.Size() * sizeof(u32));
    }

    void Mesh::ProcessNode(aiNode* node, const aiScene* scene)
    {
        u32 vertexCount = 0;
        u32 indexCount  = 0;

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

            VtCoreTrace(
                "Processing mesh: {} =>\n\tBase Vertex: {}\n\tBase Index: "
                "{}\n\tMaterial Index: {}\n\tVertex Count: {}\n\tIndex Count: "
                "{}",
                mesh->mName.C_Str(), vertexCount, indexCount,
                mesh->mMaterialIndex, mesh->mNumVertices, mesh->mNumFaces * 3);

            vertexCount += mesh->mNumVertices;
            indexCount += mesh->mNumFaces * 3;

            auto& aabb = submesh.BoundingBox;
            aabb.Min   = {FLT_MAX, FLT_MAX, FLT_MAX};
            aabb.Max   = {-FLT_MAX, -FLT_MAX, -FLT_MAX};
            for (usize i = 0; i < mesh->mNumVertices; i++)
            {
                Vertex vertex;
                vertex.Pos = {mesh->mVertices[i].x, mesh->mVertices[i].y,
                              mesh->mVertices[i].z};
                // vertex.Normal   = {mesh->mNormals[i].x, mesh->mNormals[i].y,
                //                    mesh->mNormals[i].z};
                aabb.Min.x = Min(vertex.Pos.x, aabb.Min.x);
                aabb.Min.y = Min(vertex.Pos.y, aabb.Min.y);
                aabb.Min.z = Min(vertex.Pos.z, aabb.Min.z);
                aabb.Max.x = Max(vertex.Pos.x, aabb.Max.x);
                aabb.Max.y = Max(vertex.Pos.y, aabb.Max.y);
                aabb.Max.z = Max(vertex.Pos.z, aabb.Max.z);

                if (mesh->HasTangentsAndBitangents())
                {
                    // vertex.Tangent
                    //     = {mesh->mTangents[i].x, mesh->mTangents[i].y,
                    //        mesh->mTangents[i].z};
                    // vertex.Binormal
                    //     = {mesh->mBitangents[i].x, mesh->mBitangents[i].y,
                    //        mesh->mBitangents[i].z};
                }

                // if (mesh->HasTextureCoords(0))
                //     vertex.Texcoord = {mesh->mTextureCoords[0][i].x,
                // mesh->mTextureCoords[0][i].y};

                // m_StaticVertices.push_back(vertex);
            }

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
            vertex.Color = Math::Vec3(1.0f, 1.0f, 1.0f);
            m_Vertices.PushBack(vertex);
        }

        for (u32 i = 0; i < mesh->mNumFaces; i++)
        {
            aiFace face = mesh->mFaces[i];
            for (u32 j = 0; j < face.mNumIndices; j++)
                m_Indices.PushBack(face.mIndices[j]);
        }
    }
}; // namespace Vortex
