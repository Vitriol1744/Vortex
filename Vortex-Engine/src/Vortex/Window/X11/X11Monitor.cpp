/*
 * Created by v1tr10l7 on 25.07.2024.
 * Copyright (c) 2024-2024, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#include "vtpch.hpp"

#include "Vortex/Window/X11/X11Monitor.hpp"

namespace Vortex
{
    X11Monitor::X11Monitor() {}

    Vec2i X11Monitor::GetPosition() const
    {
        VtTodo();
        return {0, 0};
    }
    Rectangle<> X11Monitor::GetWorkArea() const
    {
        VtTodo();
        return {0, 0, 1600, 900};
    }
    Vec2f X11Monitor::GetContentScale() const
    {
        VtTodo();
        return {1.0f, 1.0f};
    }
    void X11Monitor::SetGamma(f32 gamma) const
    {
        VtUnused(gamma);
        VtTodo();
    }
    void X11Monitor::SetGammaRamp(GammaRamp& gammaRamp)
    {
        VtUnused(gammaRamp);
        VtTodo();
    }

    bool X11Monitor::Initialize(std::vector<Ref<Monitor>>& monitors)
    {
        monitors.push_back(CreateRef<X11Monitor>());

        return true;
    };
}; // namespace Vortex
