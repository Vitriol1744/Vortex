//
// Created by vitriol1744 on 19.07.2021.
//
#include "Sprite.hpp"

using namespace Vortex::Math;

namespace Vortex::Graphics
{
    Sprite::Sprite()
    {
        transform = glm::mat4(1.0f);
        color = glm::vec4(1.0f, 1.0f, 0.0f, 1.0f);
    }

    void Sprite::Move(glm::vec2 offset)
    {
        Move(offset.x, offset.y);
    }

    void Sprite::Move(float32 x, float32 y)
    {
        transform = glm::translate(transform, glm::vec3(x, y, 0));}

    void Sprite::Restart()
    {
        transform = glm::mat4(1.0f);
    }
}
