/*
 * Created by v1tr10l7 on 25.08.2024.
 * Copyright (c) 2024-2024, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#include "vtpch.hpp"

#include "Vortex/Renderer/Window/Monitor.hpp"
#include "Vortex/Renderer/Window/Window.hpp"

#ifdef VT_PLATFORM_LINUX
    #include "Vortex/Renderer/Window/Wayland/WaylandMonitor.hpp"
    #include "Vortex/Renderer/Window/X11/X11Monitor.hpp"
#elifdef VT_PLATFORM_WINDOWS
    #include "Vortex/Renderer/Window/Win32/Win32Monitor.hpp"
#else
    #error "Platform not supported!"
#endif

namespace Vortex
{
    static Ref<Monitor>       s_PrimaryMonitor = nullptr;
    std::vector<Ref<Monitor>> Monitor::s_Monitors{};

    void                      Monitor::Initialize()
    {
        static bool initialized = false;
        if (initialized) return;

        VtCoreTrace("Enumerating the available monitors...");
#ifdef VT_PLATFORM_LINUX
        if (Window::GetWindowSubsystem() == WindowSubsystem::eX11)
            initialized = X11Monitor::Initialize(s_Monitors);
        else if (Window::GetWindowSubsystem() == WindowSubsystem::eWayland)
            initialized = WaylandMonitor::Initialize();
#elifdef VT_PLATFORM_WINDOWS
        initialized = Win32Monitor::Initialize(s_Monitors);
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
    const std::vector<Ref<Monitor>>& Monitor::GetMonitors()
    {
        Initialize();
        return s_Monitors;
    }

    namespace MonitorEvents
    {

        Event<Monitor*, MonitorState> MonitorStateChangedEvent;
    }; // namespace MonitorEvents
};     // namespace Vortex
