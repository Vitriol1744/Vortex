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
        explicit GL46Context(void* windowHandle, IGraphicsContext* share);
        ~GL46Context() override;

        void Present() override;
        void Activate() override;

        HWND window = 0;
        HDC deviceContext{};
        HGLRC renderingContext{};
    };
}
