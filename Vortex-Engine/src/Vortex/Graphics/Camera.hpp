//
// Created by Vitriol1744 on 07.08.2021.
//
#pragma once

#include "Vortex/Core/Core.hpp"

#include <vector>

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

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
            inline Math::Vec3 GetPosition()  const { return position; }
            inline Math::Quat GetRotation()  const { return rotation; }
            inline Math::Vec3 GetScale()     const { return scale   ; }
            inline Math::Mat4 GetProjectionMatrix() const { return projection; }
            Math::Mat4 GetViewMatrix();
            Math::Mat4 GetViewProjectionMatrix();

            inline void SetPosition(Math::Vec3 position) { this->position = position; }
            inline void SetRotation(Math::Quat rotation) { this->rotation = rotation; }
            inline void SetScale(Math::Vec3 scale) { this->scale = scale; }

        private:
            CameraProjection cameraProjection;
            Math::Vec3 position  = { 0.0f, 0.0f, 0.0f };
            Math::Quat rotation;
            Math::Vec3 scale     = { 1.0f, 1.0f, 1.0f };

            Math::Mat4 projection = glm::mat4(1.0f);
            Math::Mat4 view = glm::mat4(1.0f);
            Math::Mat4 viewProjection = glm::mat4(1.0f);
    };
}