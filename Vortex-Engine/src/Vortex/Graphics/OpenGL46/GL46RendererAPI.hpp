//
// Created by Vitriol1744 on 26.06.2021.
//
#pragma once

#include "Core/Core.hpp"

#include "Graphics/IRendererAPI.hpp"

namespace Vortex
{
    class GL46RendererAPI : public IRendererAPI
    {
        public:
            void SetClearColor(Vec4 color) override;
            void Clear() override;
    };
}


