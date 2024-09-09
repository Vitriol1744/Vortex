/*
 * Created by v1tr10l7 on 08.09.2024.
 * Copyright (c) 2024-2024, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include "Vortex/Core/Core.hpp"

namespace Vortex
{
    struct FramebufferSpecification
    {
    };

    class VT_API Framebuffer
    {
      public:
        virtual ~Framebuffer() = default;

        static Ref<Framebuffer>
        Create(const FramebufferSpecification& specification);
    };
}; // namespace Vortex
