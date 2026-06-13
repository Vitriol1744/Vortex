/*
 * Created by v1tr10l7 on 29.05.2026.
 * Copyright (c) 2024-2026, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include <Vortex/Core/Math/AABB.hpp>

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
            Vec3 dirfrac;
            // r.dir is unit direction vector of ray
            dirfrac.x        = 1.0f / Direction.x;
            dirfrac.y        = 1.0f / Direction.y;
            dirfrac.z        = 1.0f / Direction.z;
            // lb is the corner of AABB with minimal coordinates - left bottom,
            // rt is maximal corner r.org is origin of ray
            const Vec3& lb   = aabb.Min;
            const Vec3& rt   = aabb.Max;
            f32         t1   = (lb.x - Origin.x) * dirfrac.x;
            f32         t2   = (rt.x - Origin.x) * dirfrac.x;
            f32         t3   = (lb.y - Origin.y) * dirfrac.y;
            f32         t4   = (rt.y - Origin.y) * dirfrac.y;
            f32         t5   = (lb.z - Origin.z) * dirfrac.z;
            f32         t6   = (rt.z - Origin.z) * dirfrac.z;

            f32         tmin = Max(Max(Min(t1, t2), Min(t3, t4)), Min(t5, t6));
            f32         tmax = Min(Min(Max(t1, t2), Max(t3, t4)), Max(t5, t6));

            // if tmax < 0, ray (line) is intersecting AABB, but the whole AABB
            // is behind us
            if (tmax < 0)
            {
                t = tmax;
                return false;
            }

            // if tmin > tmax, ray doesn't intersect AABB
            if (tmin > tmax)
            {
                t = tmax;
                return false;
            }

            t = tmin;
            return true;
        }

        bool IntersectsTriangle(const Vec3& A, const Vec3& B, const Vec3& C,
                                f32& t)
        {
            Vec3 E1     = B - A;
            Vec3 E2     = C - A;
            Vec3 N      = glm::cross(E1, E2);
            f32  det    = -glm::dot(Direction, N);
            f32  invdet = 1.0 / det;
            Vec3 AO     = Origin - A;
            Vec3 DAO    = glm::cross(AO, Direction);
            f32  u      = glm::dot(E2, DAO) * invdet;
            f32  v      = -glm::dot(E1, DAO) * invdet;
            t           = glm::dot(AO, N) * invdet;
            return (det >= 1e-6 && t >= 0.0 && u >= 0.0 && v >= 0.0
                    && (u + v) <= 1.0);
        }
    };
}; // namespace Vortex::Math
