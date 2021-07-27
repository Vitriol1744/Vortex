//
// Created by vitriol1744 on 19.07.2021.
//
#pragma once

#include "Vortex/Core/Core.hpp"

namespace Vortex::Graphics
{
    class VT_API Sprite
    {
        public:
            Sprite();

            void Move(Math::Vec2 offset);
            void Move(float32 x, float32 y);
            void Restart();
            
            inline void SetTransform(Math::Mat4& transform) { this->transform = transform; }
            inline void SetColor(const Math::Vec4& color) { this->color = color; }

            Math::Mat4 transform;
            Math::Vec4 color;
    };
}

