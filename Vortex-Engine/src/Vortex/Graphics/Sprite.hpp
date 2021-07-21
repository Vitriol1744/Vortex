//
// Created by vitriol1744 on 19.07.2021.
//
#pragma once

#include "Core/Core.hpp"

namespace Vortex::Graphics
{
    class VT_API Sprite
    {
        public:
            Sprite();

            void Move(Math::Vec2 offset);
            void Move(float32 x, float32 y);
            void Restart();

            Math::Mat4 model;
            Math::Vec4 color;
    };
}

