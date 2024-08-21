/*
 * Created by v1tr10l7 on 19.07.2024.
 * Copyright (c) 2024-2024, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include "Vortex/Core/Core.hpp"

namespace Vortex
{
    class RendererContext
    {
      public:
        RendererContext()                      = default;
        virtual ~RendererContext()             = default;

        virtual void                Present()  = 0;
        virtual void                OnResize() = 0;

        static Ref<RendererContext> Create(class Window* window,
                                           bool          vsync = false);
    };
}; // namespace Vortex
