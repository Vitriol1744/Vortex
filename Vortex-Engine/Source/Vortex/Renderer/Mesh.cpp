/*
 * Created by v1tr10l7 on 21.09.2024.
 * Copyright (c) 2024-2026, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#include <assimp/Importer.hpp>
#include <assimp/config.h>
#include <assimp/postprocess.h>

#include <Vortex/Core/Assertions.hpp>
#include <Vortex/Core/Profiler.hpp>
#include <Vortex/Renderer/Mesh.hpp>

#include <cfloat>

import Prism.String.Formatter;
import Prism.String.StringView;
import Vortex.Renderer;

namespace Vortex
{
    namespace
    {
        glm::mat4 ToMat4(const aiMatrix4x4& m)
        {
            // assimp is row-major, glm column-major.
            return glm::mat4(m.a1, m.b1, m.c1, m.d1, m.a2, m.b2, m.c2, m.d2,
                             m.a3, m.b3, m.c3, m.d3, m.a4, m.b4, m.c4, m.d4);
        }
        String ToString(const aiString& string)
        {
            return String(string.C_Str(), string.length);
        }
    } // namespace

    Mesh::Mesh(PathView path)
        : m_FilePath(path)
    {
        VtProfileFunction();
        Assimp::Importer importer;
        // FBX pivots otherwise become extra "$AssimpFbx$" nodes the
        // animation channels target separately.
        importer.SetPropertyBool(AI_CONFIG_IMPORT_FBX_PRESERVE_PIVOTS, false);

        u32 flags = aiProcess_Triangulate | aiProcess_FlipUVs
                  | aiProcess_OptimizeMeshes | aiProcess_SortByPType
                  | aiProcess_GenSmoothNormals | aiProcess_GenUVCoords
                  | aiProcess_LimitBoneWeights | aiProcess_JoinIdenticalVertices
                  | aiProcess_ValidateDataStructure;

        const aiScene* scene = importer.ReadFile(path.Raw(), flags);
        VtCoreAssertFormat(scene && scene->HasMeshes(),
                           "Mesh: Failed to load '{}': {}", path,
                           importer.GetErrorString());

        m_AABB.Min = {FLT_MAX, FLT_MAX, FLT_MAX};
        m_AABB.Max = {-FLT_MAX, -FLT_MAX, -FLT_MAX};

        LoadSkeleton(scene);
        LoadMaterials(scene);
        ProcessNode(scene->mRootNode, scene, glm::mat4(1.0f));
        if (IsSkinned())
        {
            // Renormalize: dropped weights (past the 4 strongest) leave a gap.
            for (auto& vertex : m_Vertices)
            {
                f32 sum = vertex.BoneWeights[0] + vertex.BoneWeights[1]
                        + vertex.BoneWeights[2] + vertex.BoneWeights[3];
                if (sum > 0.0f)
                    for (f32& weight : vertex.BoneWeights) weight /= sum;
            }
            LoadAnimations(scene);
        }
        else m_Skeleton = {};

        VtCoreInfo("Mesh: '{}': {} vertices, {} bones, {} animations, extent "
                   "({}, {}, {})",
                   path, m_Vertices.Size(), m_Skeleton.Bones.Size(),
                   m_Animations.Size(), m_AABB.Max.x - m_AABB.Min.x,
                   m_AABB.Max.y - m_AABB.Min.y, m_AABB.Max.z - m_AABB.Min.z);

        auto&           device = Renderer::Device();
        RHI::BufferDesc vertices{};
        vertices.Size      = m_Vertices.Size() * sizeof(Vertex);
        vertices.Usage     = RHI::BufferUsage::eVertex;
        vertices.DebugName = Prism::Format("{} vertices", path);
        m_VertexBuffer     = device.CreateBuffer(vertices, m_Vertices.Raw());

        RHI::BufferDesc indices{};
        indices.Size      = m_Indices.Size() * sizeof(u32);
        indices.Usage     = RHI::BufferUsage::eIndex;
        indices.DebugName = Prism::Format("{} indices", path);
        m_IndexBuffer     = device.CreateBuffer(indices, m_Indices.Raw());
    }

    void Mesh::LoadMaterials(const aiScene* scene)
    {
        // Texture paths in the file are relative to it.
        StringView file(m_FilePath.Raw(), m_FilePath.Size());
        usize      slash     = file.FindLastOf('/');
        String     directory = slash == StringView::NPos ? String{} : String(file.Substr(0, slash + 1));

        for (u32 i = 0; i < scene->mNumMaterials; ++i)
        {
            const aiMaterial* source = scene->mMaterials[i];
            MeshMaterialInfo& info   = m_Materials.EmplaceBack();
            info.Name                = ToString(source->GetName());

            aiColor4D color(1.0f, 1.0f, 1.0f, 1.0f);
            if (source->Get(AI_MATKEY_BASE_COLOR, color) != aiReturn_SUCCESS)
                source->Get(AI_MATKEY_COLOR_DIFFUSE, color);
            info.Albedo[0] = color.r;
            info.Albedo[1] = color.g;
            info.Albedo[2] = color.b;
            info.Albedo[3] = color.a;

            ai_real metallic = 0.0f, roughness = 0.8f;
            if (source->Get(AI_MATKEY_METALLIC_FACTOR, metallic) == aiReturn_SUCCESS)
                info.Metallic = static_cast<f32>(metallic);
            if (source->Get(AI_MATKEY_ROUGHNESS_FACTOR, roughness) == aiReturn_SUCCESS)
                info.Roughness = static_cast<f32>(roughness);

            aiString texture;
            if (source->GetTexture(aiTextureType_BASE_COLOR, 0, &texture) != aiReturn_SUCCESS
                && source->GetTexture(aiTextureType_DIFFUSE, 0, &texture) != aiReturn_SUCCESS)
                continue;
            // Embedded textures ("*0") aren't supported.
            if (texture.length == 0 || texture.data[0] == '*') continue;
            String relative(texture.C_Str());
            info.AlbedoTexture = relative.StartsWith('/') ? Path(relative)
                                                          : Path(directory + relative);
        }
    }

    void Mesh::LoadSkeleton(const aiScene* scene)
    {
        // Every node, parents first, so posing is a single forward pass.
        auto visit = [&](auto& self, const aiNode* node, i32 parent) -> void
        {
            u32 index = m_Skeleton.Nodes.Size();
            SkeletonNode& entry = m_Skeleton.Nodes.EmplaceBack();
            entry.Name           = ToString(node->mName);
            entry.LocalTransform = ToMat4(node->mTransformation);
            entry.Parent         = parent;
            m_Skeleton.NodeIndex[entry.Name] = index;
            for (u32 i = 0; i < node->mNumChildren; ++i)
                self(self, node->mChildren[i], static_cast<i32>(index));
        };
        visit(visit, scene->mRootNode, -1);
        m_Skeleton.GlobalInverse
            = glm::inverse(ToMat4(scene->mRootNode->mTransformation));
    }

    void Mesh::LoadBones(aiMesh* mesh, u32 baseVertex)
    {
        for (u32 b = 0; b < mesh->mNumBones; ++b)
        {
            const aiBone* bone = mesh->mBones[b];
            String        name = ToString(bone->mName);

            u32 boneIndex;
            if (auto it = m_Skeleton.BoneIndex.Find(name);
                it != m_Skeleton.BoneIndex.end())
                boneIndex = it->Value;
            else
            {
                boneIndex = m_Skeleton.Bones.Size();
                VtCoreAssertFormat(boneIndex < kMaxBones,
                                   "Mesh: '{}' has more than {} bones",
                                   m_FilePath, kMaxBones);
                m_Skeleton.Bones.PushBack({name, ToMat4(bone->mOffsetMatrix)});
                m_Skeleton.BoneIndex[name] = boneIndex;
                if (auto node = m_Skeleton.NodeIndex.Find(name);
                    node != m_Skeleton.NodeIndex.end())
                    m_Skeleton.Nodes[node->Value].Bone = static_cast<i32>(boneIndex);
            }

            for (u32 w = 0; w < bone->mNumWeights; ++w)
            {
                const aiVertexWeight& weight = bone->mWeights[w];
                Vertex& vertex = m_Vertices[baseVertex + weight.mVertexId];
                // Keep the 4 strongest influences.
                u32 slot = 0;
                for (u32 i = 1; i < 4; ++i)
                    if (vertex.BoneWeights[i] < vertex.BoneWeights[slot]) slot = i;
                if (weight.mWeight > vertex.BoneWeights[slot])
                {
                    vertex.BoneIndices[slot] = boneIndex;
                    vertex.BoneWeights[slot] = weight.mWeight;
                }
            }
        }
    }

    void Mesh::LoadAnimations(const aiScene* scene)
    {
        for (u32 a = 0; a < scene->mNumAnimations; ++a)
        {
            const aiAnimation* animation = scene->mAnimations[a];
            AnimationClip&     clip      = m_Animations.EmplaceBack();
            clip.Name           = ToString(animation->mName);
            clip.Duration       = static_cast<f32>(animation->mDuration);
            clip.TicksPerSecond = animation->mTicksPerSecond > 0.0
                                    ? static_cast<f32>(animation->mTicksPerSecond)
                                    : 25.0f;

            for (u32 c = 0; c < animation->mNumChannels; ++c)
            {
                const aiNodeAnim* channel = animation->mChannels[c];
                auto node = m_Skeleton.NodeIndex.Find(ToString(channel->mNodeName));
                if (node == m_Skeleton.NodeIndex.end()) continue;

                AnimationTrack& track = clip.Tracks.EmplaceBack();
                track.Node            = node->Value;
                for (u32 k = 0; k < channel->mNumPositionKeys; ++k)
                {
                    const auto& key = channel->mPositionKeys[k];
                    track.Positions.PushBack(
                        {static_cast<f32>(key.mTime),
                         Math::Vec3(key.mValue.x, key.mValue.y, key.mValue.z)});
                }
                for (u32 k = 0; k < channel->mNumRotationKeys; ++k)
                {
                    const auto& key = channel->mRotationKeys[k];
                    track.Rotations.PushBack(
                        {static_cast<f32>(key.mTime),
                         Quat(key.mValue.w, key.mValue.x, key.mValue.y,
                              key.mValue.z)});
                }
                for (u32 k = 0; k < channel->mNumScalingKeys; ++k)
                {
                    const auto& key = channel->mScalingKeys[k];
                    track.Scales.PushBack(
                        {static_cast<f32>(key.mTime),
                         Math::Vec3(key.mValue.x, key.mValue.y, key.mValue.z)});
                }
            }
        }
    }

    void Mesh::ProcessNode(aiNode* node, const aiScene* scene,
                           const glm::mat4& parentTransform)
    {
        glm::mat4 transform = parentTransform * ToMat4(node->mTransformation);

        for (u32 i = 0; i < node->mNumMeshes; ++i)
        {
            aiMesh*  mesh         = scene->mMeshes[node->mMeshes[i]];
            Submesh& submesh      = m_Submeshes.EmplaceBack();
            submesh.BaseVertex    = m_Vertices.Size();
            submesh.BaseIndex     = m_Indices.Size();
            submesh.MaterialIndex = mesh->mMaterialIndex;
            submesh.VertexCount   = mesh->mNumVertices;
            submesh.IndexCount    = mesh->mNumFaces * 3;
            submesh.MeshName      = ToString(mesh->mName);
            submesh.NodeName      = ToString(node->mName);
            submesh.Transform     = transform;
            ProcessMesh(mesh, submesh);
            if (mesh->HasBones()) LoadBones(mesh, submesh.BaseVertex);

            // The model's bounds, with submesh node transforms applied.
            const auto& box = submesh.BoundingBox;
            for (u32 corner = 0; corner < 8; ++corner)
            {
                glm::vec3 point(corner & 1 ? box.Max.x : box.Min.x,
                                corner & 2 ? box.Max.y : box.Min.y,
                                corner & 4 ? box.Max.z : box.Min.z);
                glm::vec3 moved = glm::vec3(transform * glm::vec4(point, 1.0f));
                m_AABB.Min = glm::min(m_AABB.Min, moved);
                m_AABB.Max = glm::max(m_AABB.Max, moved);
            }
        }
        for (u32 i = 0; i < node->mNumChildren; ++i)
            ProcessNode(node->mChildren[i], scene, transform);
    }

    void Mesh::ProcessMesh(aiMesh* mesh, Submesh& submesh)
    {
        auto& box = submesh.BoundingBox;
        box.Min   = {FLT_MAX, FLT_MAX, FLT_MAX};
        box.Max   = {-FLT_MAX, -FLT_MAX, -FLT_MAX};

        for (u32 i = 0; i < mesh->mNumVertices; ++i)
        {
            Vertex vertex{};
            vertex.Position = {mesh->mVertices[i].x, mesh->mVertices[i].y,
                               mesh->mVertices[i].z};
            if (mesh->HasNormals())
                vertex.Normal = {mesh->mNormals[i].x, mesh->mNormals[i].y,
                                 mesh->mNormals[i].z};
            if (mesh->HasTextureCoords(0))
                vertex.TexCoord = {mesh->mTextureCoords[0][i].x,
                                   mesh->mTextureCoords[0][i].y};
            m_Vertices.PushBack(vertex);
            box.Min = glm::min(box.Min, vertex.Position);
            box.Max = glm::max(box.Max, vertex.Position);
        }
        // Indices address the shared vertex buffer.
        for (u32 i = 0; i < mesh->mNumFaces; ++i)
        {
            const aiFace& face = mesh->mFaces[i];
            for (u32 j = 0; j < face.mNumIndices; ++j)
                m_Indices.PushBack(submesh.BaseVertex + face.mIndices[j]);
        }
    }

    RHI::VertexBinding Mesh::StaticLayout()
    {
        RHI::VertexBinding binding{};
        binding.Stride = sizeof(Vertex);
        binding.Attributes.PushBack(
            {0, RHI::Format::eRGB32Float, static_cast<u32>(offsetof(Vertex, Position))});
        binding.Attributes.PushBack(
            {1, RHI::Format::eRGB32Float, static_cast<u32>(offsetof(Vertex, Normal))});
        binding.Attributes.PushBack(
            {2, RHI::Format::eRG32Float, static_cast<u32>(offsetof(Vertex, TexCoord))});
        return binding;
    }
    RHI::VertexBinding Mesh::SkinnedLayout()
    {
        RHI::VertexBinding binding = StaticLayout();
        binding.Attributes.PushBack({3, RHI::Format::eRGBA32Uint,
                                     static_cast<u32>(offsetof(Vertex, BoneIndices))});
        binding.Attributes.PushBack({4, RHI::Format::eRGBA32Float,
                                     static_cast<u32>(offsetof(Vertex, BoneWeights))});
        return binding;
    }
}; // namespace Vortex
