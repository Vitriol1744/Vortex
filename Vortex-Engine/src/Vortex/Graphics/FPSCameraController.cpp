//
// Created by Vitriol1744 on 13.08.2021.
//
#include "FPSCameraController.hpp"

#include "Vortex/Core/Input/Input.hpp"
#include "Vortex/Core/Time.hpp"

namespace Vortex::Graphics
{
    using namespace Math;
    using namespace Input;

    void FPSCameraController::Update()
    {
        Quat qPitch = AngleAxis(static_cast<float32>(ToRadians(-pitch)), Vec3(1, 0, 0));
        Quat qYaw = AngleAxis(static_cast<float32>(ToRadians(yaw)), Vec3(0, 1, 0));

        Quat orientation = Normalize(qPitch * qYaw);

        Vec3 forward = Rotate(Inverse(orientation), Vec3(0.0, 0.0, -1.0));
        Vec3 right = Rotate(Inverse(orientation), Vec3(1.0, 0.0, 0.0));
        Vec3 up = Vec3(0.0, 1.0, 0.0);

        camera.SetRotation(qPitch * qYaw);

        static bool firstUpdate = true;
        if (firstUpdate)
        {
            lastMousePos = Mouse::CursorPosition();
            firstUpdate = false;
        }

        Vec2 currentMousePos = Mouse::CursorPosition();
        if (Mouse::IsButtonPressed(MouseCode::eRight))
        {
            float32 xOffset = currentMousePos.x - lastMousePos.x;
            float32 yOffset = lastMousePos.y - currentMousePos.y;
            xOffset *= mouseSensitivity;
            yOffset *= mouseSensitivity;

            yaw   += xOffset * static_cast<float32>(Time::DeltaTime());
            pitch += yOffset * static_cast<float32>(Time::DeltaTime());
        }
        if (Keyboard::IsKeyPressed(KeyCode::eLeft))
        {
            yaw -= 1.0f * mouseSensitivity * static_cast<float32>(Time::DeltaTime());
        }
        else if (Keyboard::IsKeyPressed(KeyCode::eRight))
        {
            yaw += 1.0f * mouseSensitivity * static_cast<float32>(Time::DeltaTime());
        }
        lastMousePos = currentMousePos;


        static float32 speed = 2;
        if (Keyboard::IsKeyPressed(KeyCode::eLShift))
        {
            speed = 10;
        }
        else
        {
            speed = 2;
        }
        if (Keyboard::IsKeyPressed(KeyCode::eW))
        {
            camera.SetPosition(camera.GetPosition() + forward * static_cast<float32>(Time::DeltaTime()) * speed);
        }
        else if (Keyboard::IsKeyPressed(KeyCode::eS))
        {
            camera.SetPosition(camera.GetPosition() - forward * static_cast<float32>(Time::DeltaTime()) * speed);
        }
        else if (Keyboard::IsKeyPressed(KeyCode::eA))
        {
            camera.SetPosition(camera.GetPosition() - right * static_cast<float32>(Time::DeltaTime()) * speed);
        }
        else if (Keyboard::IsKeyPressed(KeyCode::eD))
        {
            camera.SetPosition(camera.GetPosition() + right * static_cast<float32>(Time::DeltaTime()) * speed);
        }
        else if (Keyboard::IsKeyPressed(KeyCode::eSpace))
        {
            camera.SetPosition(camera.GetPosition() + up * static_cast<float32>(Time::DeltaTime()) * speed);
        }
        else if (Keyboard::IsKeyPressed(KeyCode::eLCtrl))
        {
            camera.SetPosition(camera.GetPosition() - up * static_cast<float32>(Time::DeltaTime()) * speed);
        }
        if (pitch >  89.0f) pitch = 89.0f;
        if (pitch < -89.0f) pitch = -89.0f;
    }
}