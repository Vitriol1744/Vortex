/*
 * Created by v1tr10l7 on 30.08.2024.
 * Copyright (c) 2024-2024, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include <Vortex/Core/Core.hpp>

#include <X11/Xlib-xcb.h>
#include <X11/Xresource.h>
#include <X11/Xutil.h>

namespace Vortex::X11
{
    void              Initialize();
    void              Shutdown();

    Display*          Display();
    xcb_connection_t* Connection();
    i32               Screen();
    ::Window          GetRootWindow();
    XContext          GetXContext();
    XIM               InputMethod();
    Atom              Atom(StringView name, bool onlyIfExists = false);
}; // namespace Vortex::X11
