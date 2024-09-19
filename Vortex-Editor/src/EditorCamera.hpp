/*
 * Created by v1tr10l7 on 18.09.2024.
 * Copyright (c) 2024-2024, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include "Vortex/Renderer/Camera.hpp"

namespace Vortex
{
    class EditorCamera : public Camera
    {
      public:
        EditorCamera();
        void Update();

      private:
        f32  m_Yaw              = 0.0f;
        f32  m_Pitch            = 0.0f;

        f32  m_Velocity         = 1.0f;
        f32  m_MouseSensitivity = 10.0f;
        Vec2 m_LastMousePos;
    };
}; // namespace Vortex
