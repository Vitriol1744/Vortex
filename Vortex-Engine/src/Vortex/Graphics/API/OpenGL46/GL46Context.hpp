//
// Created by Vitriol1744 on 24.06.2021.
//
#pragma once

#include "Vortex/Graphics/API/IGraphicsContext.hpp"

#include "Vortex/Platform/EGL.hpp"

namespace Vortex::Graphics
{
    class VT_API GL46Context final : public IGraphicsContext
    {
        public:
            explicit GL46Context(NativeWindowHandleType windowHandle, uint32 bitsPerPixel, IGraphicsContext* share);

            virtual void Present() noexcept override;
            virtual void Activate() override;
    
        private:
            Platform::EGLContext context;
    };
}