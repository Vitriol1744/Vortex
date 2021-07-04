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
            explicit IGraphicsContext(void* windowHandle) { };
            virtual ~IGraphicsContext() = default;
            
            virtual void Present() = 0;
    };
}
