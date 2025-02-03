/*
 * Created by v1tr10l7 on 22.08.2024.
 * Copyright (c) 2024-2024, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include "Vortex/Core/Core.hpp"
#include "Vortex/Core/Math/Math.hpp"

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>

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
        Camera() = default;
        Camera(f32 left, f32 right, f32 bottom, f32 top, f32 near, f32 far);
        Camera(f32 fov, f32 aspectRatio, f32 near, f32 far);

        void SetOrthographic(f32 left, f32 right, f32 bottom, f32 top, f32 near,
                             f32 far);
        void SetPerspective(f32 fov, f32 aspectRatio, f32 near, f32 far);

        inline ProjectionType GetProjectionType() const
        {
            return m_ProjectionType;
        }
        inline const Vec3& GetPosition() const { return m_Position; }
        inline const Quat& GetRotation() const { return m_Rotation; }
        inline const Vec3& GetScale() const { return m_Scale; }
        inline const Mat4& GetProjection() const { return m_Projection; }
        inline const Mat4& GetView()
        {
            Recalculate();
            return m_View;
        }
        inline const Mat4& GetViewProjection()
        {
            Recalculate();
            return m_ViewProjection;
        }

        inline void SetPosition(const Vec3& position)
        {
            m_Dirty    = true;
            m_Position = position;
        }
        inline void SetRotation(const Quat& rotation)
        {
            m_Dirty    = true;
            m_Rotation = rotation;
        }
        inline void SetScale(const Vec3& scale)
        {
            m_Dirty = true;
            m_Scale = scale;
        }

        inline void SetProjection(const Mat4& projection)
        {
            m_Dirty      = true;
            m_Projection = projection;
        }
        inline void SetView(const Mat4& view)
        {
            m_Dirty = true;
            m_View  = view;
        }

      protected:
        ProjectionType m_ProjectionType{};
        bool           m_Dirty    = false;
        Vec3           m_Position = {0.0f, 0.0f, 0.0f};
        Quat           m_Rotation;
        Vec3           m_Scale          = {1.0f, 1.0f, 1.0f};

        Mat4           m_Projection     = Mat4(1.0f);
        Mat4           m_View           = Mat4(1.0f);
        Mat4           m_ViewProjection = Mat4(1.0f);

        void           Recalculate()
        {
            if (!m_Dirty) return;

            m_View = glm::mat4_cast(m_Rotation)
                   * glm::translate(Mat4(1.0f), -m_Position);
            m_ViewProjection = m_Projection * m_View;
            m_Dirty          = false;
        }
    };
}; // namespace Vortex
