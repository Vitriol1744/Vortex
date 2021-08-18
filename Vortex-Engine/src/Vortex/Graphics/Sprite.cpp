//
// Created by vitriol1744 on 19.07.2021.
//
#include "Sprite.hpp"

namespace Vortex::Graphics
{
    using namespace Vortex::Math;
    
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
        transform = Translate(transform, Vec3(x, y, 0));}

    void Sprite::Restart()
    {
        transform = Mat4(1.0f);
    }
}
