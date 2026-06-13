/*
 * Created by v1tr10l7 on 19.09.2024.
 * Copyright (c) 2024-2024, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#include "EditorCamera.hpp"

#include <Vortex/Engine/Application.hpp>
#include <Vortex/Window/Input/Keyboard.hpp>
#include <Vortex/Window/Input/Mouse.hpp>

#include <glm/gtx/quaternion.hpp>

namespace Vortex
{
    EditorCamera::EditorCamera()
    {
        Window& window      = Application::Get()->Window();
        f32     aspectRatio = static_cast<f32>(window.Width())
                        / static_cast<f32>(window.Height());

        SetPerspective(glm::radians(75.0f), aspectRatio, 0.1f, 100.0f);
    }

    void EditorCamera::Update()
    {
        Quat qPitch
            = glm::angleAxis(glm::radians(-m_Pitch), Math::Vec3(1, 0, 0));
        Quat qYaw = glm::angleAxis(glm::radians(-m_Yaw), Math::Vec3(0, 1, 0));
        Quat orientation        = glm::normalize(qPitch * qYaw);

        Quat inverseOrientation = glm::inverse(orientation);
        Math::Vec3 forward
            = glm::rotate(inverseOrientation, Math::Vec3(0.0f, 0.0f, -1.0f));
        Math::Vec3 right = glm::rotate(glm::inverse(orientation),
                                       Math::Vec3(1.0f, 0.0f, 0.0f));
        Math::Vec3 up    = Math::Vec3(0.0, 1.0, 0.0);

        SetRotation(qPitch * qYaw);
        Math::Vec2 currentMousePos = Mouse::Position();

        auto       cameraPos       = Position();
        f32        deltaTime       = Application::Get()->DeltaTime();

        using Input::KeyCode;
        using Input::MouseCode;
        if (Mouse::IsButtonPressed(MouseCode::eRight))
        {
            f32 xOffset = currentMousePos.x - m_LastMousePos.x;
            f32 yOffset = currentMousePos.y - m_LastMousePos.y;
            xOffset *= m_MouseSensitivity;
            yOffset *= m_MouseSensitivity;

            m_Yaw -= xOffset * deltaTime;
            m_Pitch += yOffset * deltaTime;
        }
        if (Keyboard::IsKeyPressed(KeyCode::eLeft))
            m_Yaw -= m_MouseSensitivity * deltaTime;
        else if (Keyboard::IsKeyPressed(KeyCode::eRight))
            m_Yaw += m_MouseSensitivity * deltaTime;
        m_LastMousePos = currentMousePos;

        f32 velocity   = m_Velocity;
        if (Keyboard::IsKeyPressed(KeyCode::eLShift)) velocity *= 10;
        if (Keyboard::IsKeyPressed(KeyCode::eA))
            SetPosition(cameraPos - right * deltaTime * velocity);
        else if (Keyboard::IsKeyPressed(KeyCode::eD))
            SetPosition(cameraPos + right * deltaTime * velocity);
        else if (Keyboard::IsKeyPressed(KeyCode::eW))
            SetPosition(cameraPos + forward * deltaTime * velocity);
        else if (Keyboard::IsKeyPressed(KeyCode::eS))
            SetPosition(cameraPos - forward * deltaTime * velocity);
        else if (Keyboard::IsKeyPressed(KeyCode::eSpace))
            SetPosition(cameraPos - up * deltaTime * velocity);
        else if (Keyboard::IsKeyPressed(KeyCode::eLCtrl))
            SetPosition(cameraPos + up * deltaTime * velocity);

        if (m_Pitch > 89.0f) m_Pitch = 89.0f;
        if (m_Pitch < -89.0f) m_Pitch = -89.0f;
    }
}; // namespace Vortex
