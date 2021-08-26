//
// Created by Vitriol1744 on 29.06.2021.
//
#pragma once

#include "Vortex/Core/Core.hpp"
#include "Vortex/Graphics/API/IGraphicsContext.hpp"
#include "Vortex/Platform/OpenGL.hpp"

#include "Vortex/Platform/EGL.hpp"

namespace Vortex::Graphics
{
    struct VT_API GL46Context final : public IGraphicsContext
    {
        explicit GL46Context(void* windowHandle, uint32 bitsPerPixel, IGraphicsContext* share);

        void Present() override;
        void Activate() override;

        Platform::EGLContext context;
    };
}
