//
// Created by Vitriol1744 on 24.06.2021.
//
#pragma once

#include "Core/Core.hpp"

namespace Vortex::Graphics
{
    struct VT_API IGraphicsContext
    {
        explicit IGraphicsContext(void* windowHandle) noexcept { };
        virtual ~IGraphicsContext() = default;

        virtual void Present() = 0;

        virtual void Activate() = 0;
    };
}
