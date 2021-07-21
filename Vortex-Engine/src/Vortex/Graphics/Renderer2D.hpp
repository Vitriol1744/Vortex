//
// Created by vitriol1744 on 19.07.2021.
//
#pragma once

#include "Core/Core.hpp"

#include "Graphics/Sprite.hpp"
#include "Graphics/API/IVertexArray.hpp"

#include <queue>

namespace Vortex::Graphics
{
    class VT_API Renderer2D
    {
        public:
            static void Initialize();
            static void Shutdown();

            static void DrawSprite(Sprite sprite);

            static void Submit();
    };
}