/*
 * Created by v1tr10l7 on 22.08.2024.
 * Copyright (c) 2024-2024, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#include "vtpch.hpp"

#include "Vortex/Renderer/Camera.hpp"

namespace Vortex
{

    Camera::Camera(f32 left, f32 right, f32 bottom, f32 top, f32 near, f32 far)
    {
        SetOrthographic(left, right, bottom, top, near, far);
    }
    Camera::Camera(f32 fov, f32 aspectRatio, f32 near, f32 far)
    {
        SetPerspective(fov, aspectRatio, near, far);
    }

    void Camera::SetOrthographic(f32 left, f32 right, f32 bottom, f32 top,
                                 f32 near, f32 far)
    {
        m_Projection = glm::ortho(left, right, bottom, top, near, far);
        m_Projection[1][1] *= -1;
        m_Dirty = true;
    }
    void Camera::SetPerspective(f32 fov, f32 aspectRatio, f32 near, f32 far)
    {
        m_Projection = glm::perspective(fov, aspectRatio, near, far);
        m_Projection[1][1] *= -1;
        m_Dirty = true;
    }
}; // namespace Vortex
