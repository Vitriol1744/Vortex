//
// Created by vitriol1744 on 19.07.2021.
//
#pragma once

#include "Vortex/Core/Core.hpp"

#include "Vortex/Graphics/Sprite.hpp"
#include "Vortex/Graphics/API/IVertexArray.hpp"

#include <queue>

namespace Vortex::Graphics
{
    class VT_API Renderer2D
    {
        public:
            static void Initialize();
            static void Shutdown();
            
            static void DrawSprite(Sprite sprite);
            static void DrawQuad(const Math::Vec2& position, const Math::Vec2& size, const Math::Vec4& color);
            
            static void Submit();
    };
}