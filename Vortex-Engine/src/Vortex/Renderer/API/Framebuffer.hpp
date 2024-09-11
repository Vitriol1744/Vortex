/*
 * Created by v1tr10l7 on 08.09.2024.
 * Copyright (c) 2024-2024, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include "Vortex/Core/Core.hpp"
#include "Vortex/Core/Math/Vector.hpp"

namespace Vortex
{
    struct FramebufferSpecification
    {
        bool SwapChainTarget = false;
        u32  Width           = 0;
        u32  Height          = 0;
    };

    class VT_API Framebuffer
    {
      public:
        virtual ~Framebuffer()                        = default;

        virtual Vec2u GetSize() const                 = 0;
        virtual void  OnResize(u32 width, u32 height) = 0;

        static Ref<Framebuffer>
        Create(const FramebufferSpecification& specification);
    };
}; // namespace Vortex
