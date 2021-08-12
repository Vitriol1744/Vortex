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
            inline glm::vec3 GetPosition() const { return position; }
            inline glm::vec3 GetScale() const { return scale; }
            inline glm::mat4 GetProjectionMatrix() const { return projection; }
            inline glm::mat4 GetViewMatrix() const { return view; }
            inline glm::mat4 GetViewProjectionMatrix() const { return viewProjection; }

            inline void SetPosition(glm::vec3 position) { this->position = position; Update(); }
            inline void SetScale(glm::vec3 scale) { this->scale = scale; Update(); }

        private:
            CameraProjection cameraProjection;
            bool updated = false;
            glm::vec3 position = {0.0f, 0.0f, 0.0f};
            glm::vec3 scale = { 1.0f, 1.0f, 1.0f};

            glm::mat4 projection = glm::mat4(1.0f);
            glm::mat4 view = glm::mat4(1.0f);
            glm::mat4 viewProjection = glm::mat4(1.0f);

            void Update();
            
            //TODO: static std::vector<Camera*> cameras;
    };
}