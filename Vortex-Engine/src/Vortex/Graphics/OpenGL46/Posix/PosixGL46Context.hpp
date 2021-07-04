//
// Created by Vitriol1744 on 29.06.2021.
//
#pragma once

#include "Core/Core.hpp"
#include "Graphics/IGraphicsContext.hpp"
#include "Graphics/OpenGL46/OpenGL.hpp"

namespace Vortex
{
    class VT_API GL46Context final : public IGraphicsContext
    {
        public:
        explicit GL46Context(void *windowHandle);
        
        ~GL46Context() override;

        void Present() override;

        private:
            Display* display;
            Window window;
            GLXContext context;
    };
}


