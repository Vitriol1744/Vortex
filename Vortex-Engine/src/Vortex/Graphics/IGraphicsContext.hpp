//
// Created by Vitriol1744 on 24.06.2021.
//
#pragma once

#include "Core/Core.hpp"

namespace Vortex
{
    class VT_API IGraphicsContext
    {
        public:
            IGraphicsContext() = default;
            virtual ~IGraphicsContext() = default;

            virtual void Initialize(void* windowHandle) = 0;
            virtual void SwapBuffers() = 0;
    };
}
