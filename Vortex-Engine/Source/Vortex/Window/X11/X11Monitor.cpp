/*
 * Created by v1tr10l7 on 25.07.2024.
 * Copyright (c) 2024-2024, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#include "vtpch.hpp"

#include <Vortex/Window/X11/X11Monitor.hpp>

namespace Vortex
{
    X11Monitor::X11Monitor() {}

    Math::Vec2i X11Monitor::Position() const
    {
        VtTodo();
        return {0, 0};
    }
    Rectangle<> X11Monitor::WorkArea() const
    {
        VtTodo();
        return {0, 0, 1600, 900};
    }
    Math::Vec2f X11Monitor::ContentScale() const
    {
        VtTodo();
        return {1.0f, 1.0f};
    }
    void X11Monitor::SetGamma(f32 gamma) const
    {
        VtUnused(gamma);
        VtTodo();
    }
    void X11Monitor::SetGammaRamp(struct GammaRamp& gammaRamp)
    {
        VtUnused(gammaRamp);
        VtTodo();
    }

    bool X11Monitor::Initialize(Vector<::Ref<Monitor>>& monitors)
    {
        monitors.PushBack(CreateRef<X11Monitor>());

        return true;
    };
}; // namespace Vortex
