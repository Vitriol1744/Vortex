/*
 * Created by v1tr10l7 on 25.08.2024.
 * Copyright (c) 2024-2024, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#include "vtpch.hpp"

#include "Vortex/Graphics/Window/Monitor.hpp"
#include "Vortex/Graphics/Window/Window.hpp"

#ifdef VT_PLATFORM_LINUX
    #include "Vortex/Graphics/Window/X11/X11Monitor.hpp"
#else
    #error "Platform not supported!"
#endif

namespace Vortex
{
    static Ref<Monitor>              s_PrimaryMonitor = nullptr;
    static std::vector<Ref<Monitor>> s_Monitors;
    static void                      Initialize()
    {
        static bool initialized = false;
        if (initialized) return;

        VtCoreTrace("Enumerating the available monitors...");
#ifdef VT_PLATFORM_LINUX
        if (Window::GetWindowSubsystem() == WindowSubsystem::eX11)
            initialized = X11Monitor::Initialize(s_Monitors);
#endif
        VtCoreAssert(initialized);
        VtCoreAssert(s_Monitors.size());

        s_PrimaryMonitor = s_Monitors[0];
        VtCoreInfo("Found {} monitors", s_Monitors.size());
    }

    Ref<Monitor> Monitor::GetPrimaryMonitor()
    {
        Initialize();
        return s_PrimaryMonitor;
    }
    const std::vector<Ref<Monitor>>& GetMonitors()
    {
        Initialize();
        return s_Monitors;
    }

    namespace MonitorEvents
    {

        Event<Monitor*, MonitorState> monitorStateChangedEvent;
    }; // namespace MonitorEvents
};     // namespace Vortex
