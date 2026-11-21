/*
 * Created by v1tr10l7 on 17.11.2026.
 * Copyright (c) 2024-2026, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtx/quaternion.hpp>

#include <Vortex/Core/Profiler.hpp>
#include <Vortex/Renderer/Animation.hpp>

#include <cmath>

namespace Vortex::Animation
{
    namespace
    {
        /// Index of the last key at or before `time` (keys sorted by time).
        template <typename T>
        usize FindKey(const Vector<Keyframe<T>>& keys, f32 time)
        {
            usize low = 0, high = keys.Size() - 1;
            while (low < high)
            {
                usize mid = (low + high + 1) / 2;
                if (keys[mid].Time <= time) low = mid;
                else high = mid - 1;
            }
            return low;
        }

        f32 Factor(f32 start, f32 end, f32 time)
        {
            f32 span = end - start;
            return span > 0.0f ? glm::clamp((time - start) / span, 0.0f, 1.0f)
                               : 0.0f;
        }

        Math::Vec3 Sample(const Vector<Keyframe<Math::Vec3>>& keys, f32 time,
                          const Math::Vec3& fallback)
        {
            if (keys.Empty()) return fallback;
            if (keys.Size() == 1) return keys[0].Value;
            usize i = FindKey(keys, time);
            if (i + 1 >= keys.Size()) return keys[i].Value;
            return glm::mix(keys[i].Value, keys[i + 1].Value,
                            Factor(keys[i].Time, keys[i + 1].Time, time));
        }

        Quat Sample(const Vector<Keyframe<Quat>>& keys, f32 time,
                    const Quat& fallback)
        {
            if (keys.Empty()) return fallback;
            if (keys.Size() == 1) return keys[0].Value;
            usize i = FindKey(keys, time);
            if (i + 1 >= keys.Size()) return keys[i].Value;
            return glm::normalize(
                glm::slerp(keys[i].Value, keys[i + 1].Value,
                           Factor(keys[i].Time, keys[i + 1].Time, time)));
        }

        void Accumulate(const Skeleton& skeleton, const Vector<Mat4>& locals,
                        Vector<Mat4>& outBones)
        {
            Vector<Mat4> globals(skeleton.Nodes.Size());
            outBones.Resize(skeleton.Bones.Size());
            for (usize i = 0; i < skeleton.Nodes.Size(); ++i)
            {
                const auto& node = skeleton.Nodes[i];
                globals[i] = node.Parent >= 0 ? globals[node.Parent] * locals[i]
                                              : locals[i];
                if (node.Bone >= 0)
                    outBones[node.Bone] = skeleton.GlobalInverse * globals[i]
                                        * skeleton.Bones[node.Bone].InverseBind;
            }
        }
    } // namespace

    void Evaluate(const Skeleton& skeleton, const AnimationClip& clip,
                  f32 seconds, Vector<Mat4>& outBones)
    {
        VtProfileFunction();
        f32 ticks = seconds * clip.TicksPerSecond;
        if (clip.Duration > 0.0f) ticks = std::fmod(ticks, clip.Duration);
        if (ticks < 0.0f) ticks += clip.Duration;

        Vector<Mat4> locals(skeleton.Nodes.Size());
        for (usize i = 0; i < skeleton.Nodes.Size(); ++i)
            locals[i] = skeleton.Nodes[i].LocalTransform;

        for (const auto& track : clip.Tracks)
        {
            // Unanimated components keep their bind-pose value.
            Math::Vec3 scale, translation, skew;
            Math::Vec4 perspective;
            Quat       rotation;
            glm::decompose(locals[track.Node], scale, rotation, translation, skew,
                           perspective);

            translation = Sample(track.Positions, ticks, translation);
            rotation    = Sample(track.Rotations, ticks, rotation);
            scale       = Sample(track.Scales, ticks, scale);
            locals[track.Node] = glm::translate(Mat4(1.0f), translation)
                               * glm::toMat4(rotation)
                               * glm::scale(Mat4(1.0f), scale);
        }
        Accumulate(skeleton, locals, outBones);
    }

    void BindPose(const Skeleton& skeleton, Vector<Mat4>& outBones)
    {
        Vector<Mat4> locals(skeleton.Nodes.Size());
        for (usize i = 0; i < skeleton.Nodes.Size(); ++i)
            locals[i] = skeleton.Nodes[i].LocalTransform;
        Accumulate(skeleton, locals, outBones);
    }
} // namespace Vortex::Animation
