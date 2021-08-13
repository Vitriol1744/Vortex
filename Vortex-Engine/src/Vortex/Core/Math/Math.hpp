//
// Created by Vitriol1744 on 26.06.2021.
//
#pragma once

#include "Vortex/Core/Math/MathCommon.hpp"
#include "Vortex/Core/Math/Rectangle.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace Vortex::Math
{
    using Mat4  = glm::mat4;
    using Mat3  = glm::mat3;
    using Mat2  = glm::mat2;
                
    using Vec4  = glm::vec4;
    using Vec3  = glm::vec3;
    using Vec2  = glm::vec2;
    using Vec2u = glm::vec<2, uint32>;
    using Vec2i = glm::vec<2, int32>;

    using Quaternion = glm::quat;

    inline Mat4 Translate(Mat4 mat, Vec3 translation) { return glm::translate(mat, translation); }
    inline Mat4 Rotate(Mat4 mat, float32 angle, Vec3 rotation)
    {
        return glm::rotate(mat, static_cast<float32>(ToRadians(angle)), rotation);
    }
    inline Mat4 Scale(Mat4 mat, Vec3 scale) { return glm::scale(mat, scale); }
}

template class VT_API glm::vec<4, float, glm::defaultp>;
template class VT_API glm::vec<3, float, glm::defaultp>;
template class VT_API glm::vec<2, float, glm::defaultp>;
template class VT_API glm::vec<2, Vortex::int32, glm::defaultp>;
template class VT_API glm::vec<2, Vortex::uint32, glm::defaultp>;
template class VT_API glm::mat<4, 4, glm::f32, glm::defaultp>;
