/*
 * Created by v1tr10l7 on 22.08.2024.
 * Copyright (c) 2024-2024, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include "Vortex/Core/Core.hpp"
#include "Vortex/Core/Math/Math.hpp"

namespace Vortex
{
    enum class ProjectionType
    {
        eOrthographic,
        ePerspective,
    };

    class VT_API Camera
    {
      public:
        Camera(f32 left, f32 right, f32 bottom, f32 top, f32 near, f32 far);
        Camera(f32 fov, f32 aspectRatio, f32 near, f32 far);

        void SetOrthographic(f32 left, f32 right, f32 bottom, f32 top, f32 near,
                             f32 far);
        void SetPerspective(f32 fov, f32 aspectRatio, f32 near, f32 far);

        inline ProjectionType GetProjectionType() const
        {
            return m_ProjectionType;
        }
        inline Vec3 GetPosition() const { return m_Position; }
        inline Quat GetRotation() const { return m_Rotation; }
        inline Vec3 GetScale() const { return m_Scale; }
        inline Mat4 GetProjectionMatrix() const { return m_Projection; }

        inline void SetPosition(Vec3 position) { m_Position = position; }
        inline void SetRotation(Quat rotation) { m_Rotation = rotation; }
        inline void SetScale(Vec3 scale) { m_Scale = scale; }

      private:
        ProjectionType m_ProjectionType{};
        Vec3           m_Position = {0.0f, 0.0f, 0.0f};
        Quat           m_Rotation;
        Vec3           m_Scale      = {1.0f, 1.0f, 1.0f};

        Mat4           m_Projection = Mat4(1.0f);
    };
}; // namespace Vortex
