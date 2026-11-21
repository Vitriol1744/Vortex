/*
 * Created by v1tr10l7 on 21.09.2024.
 * Copyright (c) 2024-2026, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include <assimp/scene.h>

#include <Vortex/Core/Math/AABB.hpp>
#include <Vortex/Renderer/Animation.hpp>
#include <Vortex/Renderer/Export.hpp>

import Prism.Utility.Path;
import Vortex.RHI;

namespace Vortex
{
    class Submesh
    {
      public:
        u32        BaseVertex    = 0;
        u32        BaseIndex     = 0;
        u32        MaterialIndex = 0;
        u32        IndexCount    = 0;
        u32        VertexCount   = 0;

        glm::mat4  Transform{1.0f};
        Math::AABB BoundingBox;

        String     NodeName{};
        String     MeshName{};
    };

    /// A material of the model file, as authored (textures not loaded).
    struct MeshMaterialInfo
    {
        String Name{};
        /// Absolute or relative to the working directory; empty = none.
        Path   AlbedoTexture{};
        f32    Albedo[4] = {1.0f, 1.0f, 1.0f, 1.0f};
        f32    Metallic  = 0.0f;
        f32    Roughness = 0.8f;
    };

    /**
     * A model file (anything assimp reads) as one vertex and one index
     * buffer. Skinned models also carry their skeleton and animation clips;
     * their vertices stay in bind pose and are skinned on the GPU.
     */
    class VT_RENDERER_API Mesh : public RefCounted<Mesh>
    {
      public:
        /// Vertex layout of every mesh; static meshes have zero bone weights.
        struct Vertex
        {
            Math::Vec3 Position;
            Math::Vec3 Normal;
            Math::Vec2 TexCoord;
            u32        BoneIndices[4] = {0, 0, 0, 0};
            f32        BoneWeights[4] = {0, 0, 0, 0};
        };

        explicit Mesh(PathView path);

        inline constexpr PathView FilePath() const { return m_FilePath; }

        inline ::Ref<RHI::Buffer> VertexBuffer() const { return m_VertexBuffer; }
        inline ::Ref<RHI::Buffer> IndexBuffer() const { return m_IndexBuffer; }
        inline u32                IndexCount() const { return m_Indices.Size(); }
        inline const Vector<Submesh>& Submeshes() const { return m_Submeshes; }
        /// CPU copies of the buffers (collision cooking, picking...).
        inline const Vector<Vertex>&  Vertices() const { return m_Vertices; }
        inline const Vector<u32>&     Indices() const { return m_Indices; }
        /// The file's materials; Submesh::MaterialIndex indexes them.
        inline const Vector<MeshMaterialInfo>& Materials() const { return m_Materials; }
        inline const Math::AABB&      AABB() const { return m_AABB; }

        /// Whether the vertices are bound to a skeleton.
        inline bool                   IsSkinned() const { return !m_Skeleton.Empty(); }
        inline const struct Skeleton& Skeleton() const { return m_Skeleton; }
        inline const Vector<AnimationClip>& Animations() const { return m_Animations; }

        /// Position, normal and texcoord (what static shaders read).
        static RHI::VertexBinding StaticLayout();
        /// StaticLayout plus bone indices and weights.
        static RHI::VertexBinding SkinnedLayout();

      private:
        Path                  m_FilePath;
        Vector<Vertex>        m_Vertices;
        Vector<u32>           m_Indices;
        Vector<Submesh>       m_Submeshes;
        Vector<MeshMaterialInfo> m_Materials;
        ::Ref<RHI::Buffer>    m_VertexBuffer = nullptr;
        ::Ref<RHI::Buffer>    m_IndexBuffer  = nullptr;
        Math::AABB            m_AABB;
        struct Skeleton       m_Skeleton;
        Vector<AnimationClip> m_Animations;

        void ProcessNode(aiNode* node, const aiScene* scene,
                         const glm::mat4& parentTransform);
        void ProcessMesh(aiMesh* mesh, Submesh& submesh);
        void LoadSkeleton(const aiScene* scene);
        void LoadBones(aiMesh* mesh, u32 baseVertex);
        void LoadAnimations(const aiScene* scene);
        void LoadMaterials(const aiScene* scene);
    };
}; // namespace Vortex
