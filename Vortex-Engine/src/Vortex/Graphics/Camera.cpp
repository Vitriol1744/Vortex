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
        projection = Math::Mat4::Orthographic(left, right, bottom, top, near, far);
        Update();
    }
    void Camera::CreatePerspective(float32 fov, float32 aspectRatio, float32 near, float32 far)
    {
        projection = Math::Mat4::Perspective(fov, aspectRatio, near, far);
        Update();
    }

    void Camera::Update()
    {
        Mat4 transform = Mat4::Translate(Mat4(1.0f), position);

        view = transform.Invert();
        viewProjection = projection * view;

    }
}