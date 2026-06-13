/*
 * Created by v1tr10l7 on 30.08.2024.
 * Copyright (c) 2024-2024, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#include <Prism/Containers/UnorderedMap.hpp>

#include <Vortex/Window/X11/X11.hpp>

namespace Vortex::X11
{
    namespace
    {
        ::Display*                       s_Display     = nullptr;
        xcb_connection_t*                s_Connection  = nullptr;
        i32                              s_Screen      = 0;
        ::Window                         s_RootWindow  = 0;
        XContext                         s_Context     = 0;
        XIM                              s_InputMethod = nullptr;
        UnorderedMap<StringView, ::Atom> s_AtomMap;
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

    ::Display*        Display() { return s_Display; }
    xcb_connection_t* Connection() { return s_Connection; }
    i32               Screen() { return s_Screen; }
    ::Window          GetRootWindow() { return s_RootWindow; }
    XContext          GetXContext() { return s_Context; }
    XIM               InputMethod() { return s_InputMethod; }

    ::Atom            Atom(StringView name, bool onlyIfExists)
    {
        auto it = s_AtomMap.Find(name);
        if (it != s_AtomMap.end()) return it->Value;

        ::Atom atom     = XInternAtom(s_Display, name.Raw(), onlyIfExists);
        s_AtomMap[name] = atom;

        return atom;
    }
}; // namespace Vortex::X11
