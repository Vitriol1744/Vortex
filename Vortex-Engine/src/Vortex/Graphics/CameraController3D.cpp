//
// Created by Vitriol1744 on 13.08.2021.
//
#include "CameraController3D.hpp"

#include "Vortex/Core/Input/Input.hpp"
#include "Vortex/Core/Time.hpp"

namespace Vortex::Graphics
{
    using namespace Math;
    using namespace Input;

    void CameraController3D::Update()
    {
        Quat qPitch = AngleAxis(ToRadians(-pitch), Vec3(1, 0, 0));
        Quat qYaw = AngleAxis(ToRadians(yaw), Vec3(0, 1, 0));
        Quat qRoll = AngleAxis(ToRadians(roll),Vec3(0,0,1));

        Quat orientation = Normalize(qPitch * qYaw);

        Vec3 forward = Rotate(Inverse(orientation), Vec3(0.0, 0.0, -1.0));
        Vec3 right = Rotate(Inverse(orientation), Vec3(1.0, 0.0, 0.0));
        Vec3 up = Vec3(0.0, 1.0, 0.0);

        camera.SetRotation(orientation);

        static bool firstUpdate = true;
        if (firstUpdate)
        {
            lastMousePos = Mouse::CursorPosition();
            firstUpdate = false;
        }

        Vec2 currentMousePos = Mouse::CursorPosition();
        //TODO: KeyBinds!
        if (Mouse::IsButtonPressed(MouseCode::Right))
        {
            float32 xOffset = currentMousePos.x - lastMousePos.x;
            float32 yOffset = lastMousePos.y - currentMousePos.y;
            xOffset *= mouseSensitivity;
            yOffset *= mouseSensitivity;

            yaw   += xOffset * Time::DeltaTime();
            pitch += yOffset * Time::DeltaTime();
        }
        if (Keyboard::IsKeyPressed(KeyCode::Left))
        {
            yaw -= 1.0f * mouseSensitivity * Time::DeltaTime();
        }
        else if (Keyboard::IsKeyPressed(KeyCode::Right))
        {
            yaw += 1.0f * mouseSensitivity * Time::DeltaTime();
        }
        lastMousePos = currentMousePos;


        static float32 speed = 2;
        if (Keyboard::IsKeyPressed(KeyCode::LShift))
        {
            speed = 10;
        }
        else
        {
            speed = 2;
        }
        if (Keyboard::IsKeyPressed(KeyCode::W))
        {
            camera.SetPosition(camera.GetPosition() + forward * (float32)Time::DeltaTime() * speed);
        }
        else if (Keyboard::IsKeyPressed(KeyCode::S))
        {
            camera.SetPosition(camera.GetPosition() - forward * (float32)Time::DeltaTime() * speed);
        }
        else if (Keyboard::IsKeyPressed(KeyCode::A))
        {
            camera.SetPosition(camera.GetPosition() - right * (float32)Time::DeltaTime() * speed);
        }
        else if (Keyboard::IsKeyPressed(KeyCode::D))
        {
            camera.SetPosition(camera.GetPosition() + right * (float32)Time::DeltaTime() * speed);
        }
        else if (Keyboard::IsKeyPressed(KeyCode::Space))
        {
            camera.SetPosition(camera.GetPosition() + up * (float32)Time::DeltaTime() * speed);
        }
        else if (Keyboard::IsKeyPressed(KeyCode::LCtrl))
        {
            camera.SetPosition(camera.GetPosition() - up * (float32)Time::DeltaTime() * speed);
        }
        if (pitch >  89.0f) pitch = 89.0f;
        if (pitch < -89.0f) pitch = -89.0f;
    }
}