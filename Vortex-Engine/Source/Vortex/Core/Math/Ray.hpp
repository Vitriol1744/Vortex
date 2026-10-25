/*
 * Created by v1tr10l7 on 29.05.2026.
 * Copyright (c) 2024-2026, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include <Prism/Utility/Math.hpp>
#include <Vortex/Core/Math/AABB.hpp>
#include <Vortex/Core/Math/Matrix.hpp>

namespace Vortex::Math
{
    struct Ray
    {
        Vec3 Origin;
        Vec3 Direction;

        Ray()
            : Origin(Vec3(0))
            , Direction(Vec3(0))
        {
        }

        Ray(const Vec3& origin, const Vec3& direction)
            : Origin(origin)
            , Direction(direction)
        {
        }

        bool IntersectsAABB(const AABB& aabb, f32& t) const
        {
            f32 tmin = -std::numeric_limits<f32>::infinity();
            f32 tmax = std::numeric_limits<f32>::infinity();

            for (int i = 0; i < 3; ++i)
            {
                if (Prism::Math::Absolute(Direction[i]) < 1e-8f)
                {
                    // Ray is parallel to slab; fail immediately if origin is
                    // outside bounds
                    if (Origin[i] < aabb.Min[i] || Origin[i] > aabb.Max[i])
                        return false;
                }
                else {
                    f32 invDir = 1.0f / Direction[i];
                    f32 t1     = (aabb.Min[i] - Origin[i]) * invDir;
                    f32 t2     = (aabb.Max[i] - Origin[i]) * invDir;

                    if (t1 > t2) Swap(t1, t2);

                    tmin = Max(tmin, t1);
                    tmax = Min(tmax, t2);

                    if (tmin > tmax) return false;
                }
            }

            if (tmax < 0.0f) return false;

            t = (tmin < 0.0f) ? 0.0f : tmin;
            return true;
        }

        bool IntersectsTriangle(const Vec3& A, const Vec3& B, const Vec3& C,
                                f32& t, bool cullBackfaces = true) const
        {
            Vec3 E1  = B - A;
            Vec3 E2  = C - A;
            Vec3 N   = glm::cross(E1, E2);
            f32  det = -glm::dot(Direction, N);

            if (cullBackfaces && det < 1e-6f) return false;

            if (!cullBackfaces && Prism::Math::Absolute(det) < 1e-6f)
                return false;

            f32  invdet = 1.0f / det;
            Vec3 AO     = Origin - A;
            Vec3 DAO    = glm::cross(AO, Direction);

            f32  u      = glm::dot(E2, DAO) * invdet;
            if (u < 0.0f || u > 1.0f) return false;

            f32 v = -glm::dot(E1, DAO) * invdet;
            if (v < 0.0f || (u + v) > 1.0f) return false;

            t = glm::dot(AO, N) * invdet;
            return t >= 0.0f;
        }
    };
}; // namespace Vortex::Math
