//
// Created by Vitriol1744 on 29.06.2021.
//
#pragma once

#include "Core/Core.hpp"
#include "Graphics/API/IGraphicsContext.hpp"
#include "Graphics/API/OpenGL46/OpenGL.hpp"

namespace Vortex::Graphics
{
    struct VT_API GL46Context final : public IGraphicsContext
    {
        explicit GL46Context(GLvoid* windowHandle, IGraphicsContext* share);
        
        ~GL46Context() noexcept override;

        void Present() noexcept override;
        void Activate() override;

        Display* display;
        Window window;
        GLXContext context;
        static GLboolean initialized;
    };
}


