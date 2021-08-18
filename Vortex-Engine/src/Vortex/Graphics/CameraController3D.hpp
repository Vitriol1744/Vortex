//
// Created by Vitriol1744 on 13.08.2021.
//
#pragma once

#include "Vortex/Core/Core.hpp"

#include "Vortex/Graphics/Camera.hpp"

namespace Vortex::Graphics
{
    class CameraController3D
    {
        public:
            CameraController3D() { }

            void Update();

            Camera& GetCamera() { return camera; }

        private:
            Camera camera;
            Math::Vec2 lastMousePos = { 0.0f, 0.0f };
            float32 mouseSensitivity   = 150.0f;

            float32 pitch = 0.0f;
            float32 yaw   = 0.0f;
            float32 roll = 0.0f;

            Math::Vec3 front;
            Math::Vec3 up;
            Math::Vec3 right;
    };
}

