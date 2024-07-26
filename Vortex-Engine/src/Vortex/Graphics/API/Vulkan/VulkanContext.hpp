/*
 * Created by v1tr10l7 on 19.07.2024.
 * Copyright (c) 2024-2024, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include "Vortex/Graphics/API/RendererContext.hpp"

namespace Vortex
{
    class VulkanContext : public RendererContext
    {
      public:
        VulkanContext(class Window* window) { (void)window; }
        virtual ~VulkanContext() {}

        virtual void Present() override {}
        virtual void OnResize(u32 width, u32 height) override
        {
            (void)width;
            (void)height;
        }
    };
}; // namespace Vortex
