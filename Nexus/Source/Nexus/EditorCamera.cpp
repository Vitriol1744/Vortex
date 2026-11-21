/*
 * Created by v1tr10l7 on 19.09.2024.
 * Copyright (c) 2024-2024, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#include <Nexus/EditorCamera.hpp>

#include <Vortex/Asset/AssetManager.hpp>
#include <Vortex/Engine/Application.hpp>
#include <Vortex/Renderer/RenderTargetWindow.hpp>
#include <Vortex/Window/Input/Keyboard.hpp>
#include <Vortex/Window/Input/Mouse.hpp>

#include <glm/gtx/quaternion.hpp>

namespace Vortex
{
    EditorCamera::EditorCamera()
    {
        auto& window      = Application::Get()->Window();
        f32   aspectRatio = static_cast<f32>(window.Width())
                        / static_cast<f32>(window.Height());

        SetPerspective(glm::radians(75.0f), aspectRatio, 0.1f, 100.0f);
    }

    void EditorCamera::Update(const EditorCameraControls& controls)
    {
        Quat qPitch
            = glm::angleAxis(glm::radians(-m_Pitch), Math::Vec3(1, 0, 0));
        Quat qYaw = glm::angleAxis(glm::radians(-m_Yaw), Math::Vec3(0, 1, 0));
        Quat orientation        = glm::normalize(qPitch * qYaw);

        Quat inverseOrientation = glm::inverse(orientation);
        Math::Vec3 forward
            = glm::rotate(inverseOrientation, Math::Vec3(0.0f, 0.0f, -1.0f));
        Math::Vec3 right
            = glm::rotate(inverseOrientation, Math::Vec3(1.0f, 0.0f, 0.0f));
        Math::Vec3 up = Math::Vec3(0.0, 1.0, 0.0);

        SetRotation(qPitch * qYaw);
        Math::Vec2 currentMousePos = Mouse::Position();
        f32        deltaTime       = Application::Get()->DeltaTime();
        auto down = [](i32 key) { return key && ImGui::IsKeyDown(static_cast<ImGuiKey>(key)); };

        using Input::MouseCode;
        f32 sensitivity = controls.LookSensitivity;
        if (Mouse::IsButtonPressed(MouseCode::eRight))
        {
            f32 xOffset = (currentMousePos.x - m_LastMousePos.x) * sensitivity;
            f32 yOffset = (currentMousePos.y - m_LastMousePos.y) * sensitivity;
            if (controls.InvertY) yOffset = -yOffset;

            m_Yaw -= xOffset * deltaTime;
            m_Pitch += yOffset * deltaTime;
        }
        if (down(controls.TurnLeft)) m_Yaw -= sensitivity * deltaTime;
        if (down(controls.TurnRight)) m_Yaw += sensitivity * deltaTime;
        m_LastMousePos = currentMousePos;

        // Each key adds its direction, so diagonals work.
        Math::Vec3 move{0.0f};
        if (down(controls.Forward)) move += forward;
        if (down(controls.Backward)) move -= forward;
        if (down(controls.Right)) move += right;
        if (down(controls.Left)) move -= right;
        // The engine's camera space has Y down: "up" moves along -Y.
        if (down(controls.Up)) move -= up;
        if (down(controls.Down)) move += up;

        f32 velocity = controls.MoveSpeed;
        if (down(controls.Fast)) velocity *= controls.FastMultiplier;
        if (move != Math::Vec3(0.0f))
            SetPosition(Position() + move * deltaTime * velocity);

        if (m_Pitch > 89.0f) m_Pitch = 89.0f;
        if (m_Pitch < -89.0f) m_Pitch = -89.0f;
    }
}; // namespace Vortex
