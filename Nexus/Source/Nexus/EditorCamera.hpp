/*
 * Created by v1tr10l7 on 18.09.2024.
 * Copyright (c) 2024-2024, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include <Vortex/Scene/Camera.hpp>

#include <imgui.h>

namespace Vortex
{
    /// How the editor camera flies: keys are ImGuiKey values, so they can be
    /// rebound in the settings (Input > Camera Controls).
    struct EditorCameraControls
    {
        i32  Forward         = ImGuiKey_W;
        i32  Backward        = ImGuiKey_S;
        i32  Left            = ImGuiKey_A;
        i32  Right           = ImGuiKey_D;
        i32  Up              = ImGuiKey_Space;
        i32  Down            = ImGuiKey_LeftCtrl;
        i32  Fast            = ImGuiKey_LeftShift;
        i32  TurnLeft        = ImGuiKey_LeftArrow;
        i32  TurnRight       = ImGuiKey_RightArrow;

        /// Units per second, and the multiplier while Fast is held.
        f32  MoveSpeed       = 1.0f;
        f32  FastMultiplier  = 10.0f;
        /// Degrees per pixel per second of right-drag.
        f32  LookSensitivity = 10.0f;
        bool InvertY         = false;
    };

    class EditorCamera : public Camera
    {
      public:
        EditorCamera();
        void Update(const EditorCameraControls& controls);

      private:
        f32        m_Yaw   = 0.0f;
        f32        m_Pitch = 0.0f;
        Math::Vec2 m_LastMousePos;
    };
}; // namespace Vortex
