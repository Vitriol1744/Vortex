//
// Created by Vitriol1744 on 07.08.2021.
//
#pragma once

#include "Vortex/Core/Core.hpp"

#include <vector>

namespace Vortex::Graphics
{
    enum class CameraProjection
    {
        Orthographic,
        Perspective
    };
    
    class VT_API Camera
    {
        public:
            inline Camera() = default;
            Camera(float32 left, float32 right, float32 bottom, float32 top, float32 near, float32 far);
            Camera(float32 fov, float32 aspectRatio, float32 near, float32 far);

            void CreateOrthographic(float32 left, float32 right, float32 bottom, float32 top, float32 near, float32 far);
            void CreatePerspective(float32 fov, float32 aspectRatio, float32 near, float32 far);

            inline CameraProjection GetProjection() const { return cameraProjection; }
            inline Math::Vec3 GetPosition() const { return position; }
            inline Math::Mat4 GetProjectionMatrix() const { return projection; }
            inline Math::Mat4 GetViewMatrix() const { return view; }
            inline Math::Mat4 GetViewProjectionMatrix() const { return viewProjection; }

            inline void SetPosition(Math::Vec3 position) { this->position = position; Update(); }
            
        private:
            CameraProjection cameraProjection;
            bool updated = false;
            Math::Vec3 position = { 0, 0, 0 };

            Math::Mat4 projection;
            Math::Mat4 view;
            Math::Mat4 viewProjection;

            void Update();
            
            //TODO: static std::vector<Camera*> cameras;
    };
}