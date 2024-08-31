/*
 * Created by v1tr10l7 on 30.08.2024.
 * Copyright (c) 2024-2024, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#include "vtpch.hpp"

#include "Vortex/Renderer/Window/X11/X11.hpp"

namespace Vortex::X11
{
    namespace
    {
        Display*                                   s_Display     = nullptr;
        xcb_connection_t*                          s_Connection  = nullptr;
        i32                                        s_Screen      = 0;
        ::Window                                   s_RootWindow  = 0;
        XContext                                   s_Context     = 0;
        XIM                                        s_InputMethod = nullptr;
        std::unordered_map<std::string_view, Atom> s_AtomMap;
    }; // namespace

    void Initialize()
    {
        VtCoreTrace("X11: Initializing...");

        XInitThreads();
        XrmInitialize();

        const char* displayEnv = std::getenv("DISPLAY");
        VtCoreAssertMsg(displayEnv, "$DISPLAY environment variable is not set");

        s_Display = XOpenDisplay(displayEnv);
        if (!s_Display)
        {
            VtCoreAssertFormat(false, "X11: Failed to open display {}",
                               displayEnv);
        }
        s_Connection = XGetXCBConnection(s_Display);
        VtCoreAssert(s_Connection);

        s_Screen      = DefaultScreen(s_Display);
        s_RootWindow  = DefaultRootWindow(s_Display);
        s_Context     = XUniqueContext();
        s_InputMethod = XOpenIM(s_Display, nullptr, nullptr, nullptr);
    }
    void Shutdown()
    {
        VtCoreTrace("X11: Shutting down...");
        XCloseIM(s_InputMethod);
        XCloseDisplay(s_Display);
        // XFreeThreads();
    }

    Display*          GetDisplay() { return s_Display; }
    xcb_connection_t* GetConnection() { return s_Connection; }
    i32               GetScreen() { return s_Screen; }
    ::Window          GetRootWindow() { return s_RootWindow; }
    XContext          GetXContext() { return s_Context; }
    XIM               GetInputMethod() { return s_InputMethod; }

    Atom              GetAtom(std::string_view name, bool onlyIfExists)
    {
        const auto it = s_AtomMap.find(name);
        if (it != s_AtomMap.end()) return it->second;

        Atom atom       = XInternAtom(s_Display, name.data(), onlyIfExists);
        s_AtomMap[name] = atom;

        return atom;
    }
}; // namespace Vortex::X11
