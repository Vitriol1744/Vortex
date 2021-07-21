//
// Created by vitriol1744 on 19.07.2021.
//
#include "Sprite.hpp"

namespace Vortex::Graphics
{
    Sprite::Sprite()
    {
        model = Math::Mat4(1.0f);
        color = Math::Vec4(1.0f, 1.0f, 0.0f, 1.0f);
    }

    void Sprite::Move(Math::Vec2 offset)
    {
        Move(offset.x, offset.y);
    }

    void Sprite::Move(float32 x, float32 y)
    {
        model = model * Math::Mat4::Translate(Math::Vec3(x, y, 0));}

    void Sprite::Restart()
    {
        model = Math::Mat4(1.0f);
    }
}
