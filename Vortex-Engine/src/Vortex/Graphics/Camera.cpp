//
// Created by Vitriol1744 on 07.08.2021.
//
#include "Camera.hpp"

namespace Vortex::Graphics
{
    using namespace Math;

    Camera::Camera(float32 left, float32 right, float32 bottom, float32 top, float32 near, float32 far)
    {
        CreateOrthographic(left, right, bottom, top, near, far);
    }
    Camera::Camera(float32 fov, float32 aspectRatio, float32 near, float32 far)
    {
        CreatePerspective(fov, aspectRatio, near, far);
    }

    void Camera::CreateOrthographic(float32 left, float32 right, float32 bottom, float32 top, float32 near, float32 far)
    {
        projection = glm::ortho(left, right, bottom, top, near, far);
        Update();
    }
    void Camera::CreatePerspective(float32 fov, float32 aspectRatio, float32 near, float32 far)
    {
        projection = glm::perspective(fov, aspectRatio, near, far);
        Update();
    }

    void Camera::Update()
    {
        glm::mat4 transform = glm::translate(glm::mat4(1.0f), position)
                * glm::scale(glm::mat4(1.0f), scale);

        view = glm::inverse(transform);
        viewProjection = projection * view;
    }
}