//
// Created by Vitriol1744 on 24.06.2021.
//
#pragma once

#include "Core/Core.hpp"
#include "Graphics/IGraphicsContext.hpp"
#include "OpenGL.hpp"

namespace Vortex
{
    class GL46Context : public IGraphicsContext
    {
        public:
            GL46Context() = default;

            void Initialize(void* windowHandle) override;
            void SwapBuffers() override;

        private:
            #ifdef VT_PLATFORM_WINDOWS
            HDC deviceContext{};
            HGLRC renderingContext{};
            #endif
    };
}


