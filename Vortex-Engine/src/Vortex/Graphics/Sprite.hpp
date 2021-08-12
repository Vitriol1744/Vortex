//
// Created by vitriol1744 on 19.07.2021.
//
#pragma once

#include "Vortex/Core/Core.hpp"

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

namespace Vortex::Graphics
{
    class VT_API Sprite
    {
        public:
            Sprite();

            void Move(glm::vec2 offset);
            void Move(float32 x, float32 y);
            void Restart();
            
            inline void SetTransform(glm::mat4& transform) { this->transform = transform; }
            inline void SetColor(const glm::vec4& color) { this->color = color; }

            glm::mat4 transform;
            glm::vec4 color;
    };
}

