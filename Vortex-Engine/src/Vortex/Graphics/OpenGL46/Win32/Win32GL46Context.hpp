//
// Created by Vitriol1744 on 29.06.2021.
//
#pragma once

#include "Core/Core.hpp"
#include "Graphics/IGraphicsContext.hpp"
#include "Graphics/OpenGL46/OpenGL.hpp"

namespace Vortex
{
    class VT_API GL46Context : public IGraphicsContext
    {
        public:
            GL46Context() = default;
            ~GL46Context() override;

            void Initialize(void* windowHandle) override;
            void Present() override;

        private:
            HWND window{};
            HDC deviceContext{};
            HGLRC renderingContext{};
    };
}
