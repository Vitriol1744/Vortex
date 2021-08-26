//
// Created by Vitriol1744 on 24.06.2021.
//
#pragma once

#include "Vortex/Graphics/API/IGraphicsContext.hpp"

#include "Vortex/Platform/EGL.hpp"

namespace Vortex::Graphics
{
    struct VT_API GL46Context final : public IGraphicsContext
    {
        explicit GL46Context(void* windowHandle, IGraphicsContext* share);

        void Present() noexcept override;
        void Activate() override;

        Platform::EGLContext context;
    };
}