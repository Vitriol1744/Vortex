//
// Created by vitriol1744 on 19.07.2021.
//
#include "Sprite.hpp"

using namespace Vortex::Math;

namespace Vortex::Graphics
{
    Sprite::Sprite()
    {
        transform = Mat4(1.0f);
        color = Vec4(1.0f, 1.0f, 0.0f, 1.0f);
    }

    void Sprite::Move(Vec2 offset)
    {
        Move(offset.x, offset.y);
    }

    void Sprite::Move(float32 x, float32 y)
    {
        transform = Mat4::Translate(transform, Math::Vec3(x, y, 0));}

    void Sprite::Restart()
    {
        transform = Math::Mat4(1.0f);
    }
}
