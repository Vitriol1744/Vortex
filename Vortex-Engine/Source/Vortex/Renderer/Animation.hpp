/*
 * Created by v1tr10l7 on 17.11.2026.
 * Copyright (c) 2024-2026, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

// A header rather than a module: these types are glm through and through,
// which clang 23 fails to merge across module boundaries (see SceneRenderer).

#include <Vortex/Core/Math/Matrix.hpp>
#include <Vortex/Core/Math/Quaternion.hpp>
#include <Vortex/Core/Math/Vector.hpp>
#include <Vortex/Core/Types.hpp>
#include <Vortex/Renderer/Export.hpp>

import Prism.Containers.UnorderedMap;
import Prism.Containers.Vector;
import Prism.String.String;
import Prism.String.StringView;

namespace Vortex
{
    /// Most bones a skinned mesh may have (size of a pose buffer).
    inline constexpr u32 kMaxBones = 256;

    struct SkeletonNode
    {
        String Name{};
        /// Transform relative to the parent in the bind pose.
        Mat4   LocalTransform{1.0f};
        /// Index of the parent node, -1 for the root. Parents always come
        /// before their children.
        i32    Parent = -1;
        /// Bone driven by this node, -1 if none.
        i32    Bone   = -1;
    };

    struct Bone
    {
        String Name{};
        /// Mesh space -> bone space in the bind pose (assimp's offset matrix).
        Mat4   InverseBind{1.0f};
    };

    struct Skeleton
    {
        Vector<SkeletonNode>      Nodes;
        Vector<Bone>              Bones;
        /// Inverse of the root node's transform.
        Mat4                      GlobalInverse{1.0f};
        UnorderedMap<String, u32> NodeIndex;
        UnorderedMap<String, u32> BoneIndex;

        inline bool               Empty() const { return Bones.Empty(); }
    };

    template <typename T>
    struct Keyframe
    {
        f32 Time = 0.0f; // in ticks
        T   Value{};
    };

    /// The animation of one node.
    struct AnimationTrack
    {
        u32                        Node = 0;
        Vector<Keyframe<Math::Vec3>> Positions;
        Vector<Keyframe<Quat>>       Rotations;
        Vector<Keyframe<Math::Vec3>> Scales;
    };

    struct AnimationClip
    {
        String                 Name{};
        f32                    Duration       = 0.0f; // in ticks
        f32                    TicksPerSecond = 25.0f;
        Vector<AnimationTrack> Tracks;

        inline f32 DurationSeconds() const { return Duration / TicksPerSecond; }
    };

    namespace Animation
    {
        /**
         * Poses `skeleton` with `clip` at `seconds` (looping) and writes the
         * skinning matrix of every bone (mesh space -> animated mesh space)
         * into `outBones`.
         */
        VT_RENDERER_API void Evaluate(const Skeleton&      skeleton,
                                      const AnimationClip& clip, f32 seconds,
                                      Vector<Mat4>&        outBones);
        /// Bind-pose skinning matrices (identity skinning).
        VT_RENDERER_API void BindPose(const Skeleton& skeleton,
                                      Vector<Mat4>&   outBones);
    } // namespace Animation
} // namespace Vortex
