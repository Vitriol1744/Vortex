/*
 * Created by v1tr10l7 on 18.07.2024.
 * Copyright (c) 2024-2024, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#include <Prism/String/StringUtils.hpp>
#include <Vortex/Core/Platform.hpp>
#include <Vortex/Window/Window.hpp>

#ifdef VT_PLATFORM_LINUX
    #include <Vortex/Window/Wayland/WaylandWindow.hpp>
    #include <Vortex/Window/X11/X11Window.hpp>
    #define VT_NFD_WINDOW_HANDLE_TYPE NFD_WINDOW_HANDLE_TYPE_X11
#elifdef VT_PLATFORM_WINDOWS
    #include <Vortex/Window/Win32/Win32Window.hpp>
    #define VT_NFD_WINDOW_HANDLE_TYPE NFD_WINDOW_HANDLE_TYPE_WINDOWS
#elifdef VT_PLATFORM_OSX
    #define VT_NFD_WINDOW_HANDLE_TYPE NFD_WINDOW_HANDLE_TYPE_COCOA
#endif

#ifndef VT_PLATFORM_ANDROID
    #include <nfd.hpp>
#endif

namespace Vortex
{
    static enum WindowSubsystem ChooseSubsystem()
    {
#ifdef VT_PLATFORM_LINUX
        bool        waylandAvailable = ::std::getenv("WAYLAND_DISPLAY");
        bool        xlibAvailable    = ::std::getenv("DISPLAY");

        const char* platformOverride = ::std::getenv("VT_PLATFORM_OVERRIDE");
        if (platformOverride)
        {
            if (strcmp(platformOverride, "wayland") == 0 && waylandAvailable)
                return WindowSubsystem::eWayland;
            else if (strcmp(platformOverride, "x11") == 0 && xlibAvailable)
                return WindowSubsystem::eX11;
            else
                VtCoreWarn(
                    "Unrecognized platform override, VT_PLATFORM_OVERRIDE='{}'",
                    platformOverride);
        }
        else if (waylandAvailable) return WindowSubsystem::eWayland;
        else if (xlibAvailable) return WindowSubsystem::eX11;

        VtCoreAssertMsg(false, "no supported window servers detected");
        return WindowSubsystem::eUndefined;
#elifdef VT_PLATFORM_WINDOWS
        return WindowSubsystem::eWin32;
#elifdef VT_PLATFORM_ANDROID
        return WindowSubsystem::eAndroidSurface;
#else
    #error "Platform not supported!"
#endif
    };

#ifndef VT_PLATFORM_ANDROID
    static void ProcessFilters(Span<const Window::FileFilter> filters,
                               Vector<String>&                outStorage,
                               Vector<nfdu8filteritem_t>&     outNfdItems)
    {
        if (filters.Empty()) return;

        // Reserve space for both strings (description + extensions) up-front
        // CRITICAL: Vector MUST NOT reallocate after taking .c_str() pointers!
        outStorage.Reserve(filters.Size() * 2);
        outNfdItems.Reserve(filters.Size());

        for (const auto& f : filters)
        {
            outStorage.EmplaceBack(f.Description);
            const char* namePtr = outStorage.Back().Raw();

            outStorage.EmplaceBack(f.Extensions);
            const char* extPtr = outStorage.Back().Raw();

            outNfdItems.PushBack({namePtr, extPtr});
        }
    }
    static nfdwindowhandle_t
    GetNativeHandle(PM_UNUSED const Window* parentWindow)
    {
        nfdwindowhandle_t parentHandle{};
    #if !defined(VT_PLATFORM_LINUX)
        if (parentWindow)
        {
            parentHandle.type = VT_NFD_WINDOW_HANDLE_TYPE;
            // TODO(v1tr10l7): Bind parentHandle.handle from parentWindow
        }
    #else
        if (parentWindow
            && Window::WindowSubsystem() != WindowSubsystem::eWayland)
        {
            parentHandle.type = VT_NFD_WINDOW_HANDLE_TYPE;
            // TODO(v1tr10l7): Bind parentHandle.handle from parentWindow
        }
    #endif
        return parentHandle;
    }
#endif

    EventBus* Window::s_EventBus = nullptr;
    static ::Delegate<Window*(const WindowSpecification&)> s_CreateWindow;
    static ::Delegate<void()>                              s_PollEvents;
    static ::Delegate<void()>                              s_Shutdown;

    Window::Window(const WindowSpecification& specs)
    {
        m_Data.VideoMode        = specs.VideoMode;
        m_Data.Title            = specs.Title;
        m_Data.Position         = specs.Position;
        m_Data.IsOpen           = true;
        m_Data.Fullscreen       = specs.Fullscreen;
        m_Data.Resizable        = specs.Resizable;
        m_Data.Visible          = specs.Visible;
        m_Data.Focused          = specs.Focused;
        m_Data.AlwaysOnTop      = specs.AlwaysOnTop;
        m_Data.MouseHovered     = false;
        m_Data.Decorated        = specs.Decorated;
        m_Data.FocusOnShow      = specs.FocusOnShow;
        m_Data.AutoIconify      = specs.AutoIconify;
        m_Data.MousePassthrough = false;
    }

    void Window::Initialize(EventBus* eventBus)
    {
        s_EventBus = eventBus;
#ifdef VT_PLATFORM_LINUX
        if (WindowSubsystem() == WindowSubsystem::eX11)
        {
            X11::Initialize();
            s_CreateWindow = [](const WindowSpecification& specs) -> Window*
            { return new X11Window(specs); };
            s_PollEvents.Bind<X11Window::PollEvents>();
            s_Shutdown.Bind<X11::Shutdown>();
        }
        else if (WindowSubsystem() == WindowSubsystem::eWayland)
        {
            WaylandWindow::Initialize();
            s_CreateWindow = [](const WindowSpecification& specs) -> Window*
            { return new WaylandWindow(specs); };
            s_PollEvents.Bind<WaylandWindow::PollEvents>();
            s_Shutdown.Bind<Wayland::Shutdown>();
        }
#elifdef VT_PLATFORM_WINDOWS
        s_CreateWindow = [](const WindowSpecification& specs) -> Window*
        { return new Win32Window(specs); };
        s_PollEvents.Bind<Win32Window::PollEvents>();
        s_Shutdown.Bind<Win32Window::Shutdown>();
#endif

#ifndef VT_PLATFORM_ANDROID
        NFD_Init();
#endif
        VtCoreAssertMsg(s_CreateWindow && s_PollEvents && s_Shutdown,
                        "Unsupported platform!");
    }
    void Window::Shutdown()
    {
#ifndef VT_PLATFORM_ANDROID
        NFD_Quit();
#endif

        s_Shutdown();
    }

    void Window::PollEvents() { s_PollEvents(); }

#ifndef VT_PLATFORM_ANDROID
    Optional<Path> Window::OpenFile(Span<const Window::FileFilter> filters,
                                    PathView                       initialPath,
                                    const Window*                  parentWindow,
                                    StringView                     dialogTitle)
    {
        NFD::Guard                nfdGuard;

        Vector<String>            filterStorage;
        Vector<nfdu8filteritem_t> nfdFilters;
        ProcessFilters(filters, filterStorage, nfdFilters);

        // NOTE(v1tr10l7): We're copying the initialPath to a new variable of
        // type Path to ensure, that it gets null-terminated, so nfd doesn't
        // fail
        Path                    defaultPath  = initialPath;
        const nfdwindowhandle_t parentHandle = GetNativeHandle(parentWindow);

        NFD::UniquePath         outPath;
        nfdresult_t             result = NFD::OpenDialog(
            outPath, nfdFilters.Empty() ? nullptr : nfdFilters.Raw(),
            static_cast<nfdfiltersize_t>(nfdFilters.Size()),
            !defaultPath.Empty() ? defaultPath.Raw() : nullptr, parentHandle);

        return (result == NFD_OKAY) ? Optional<Path>(Path(outPath.get()))
                                    : NullOpt;
    }
    Vector<Path>
    Window::OpenMultipleFiles(Span<const Window::FileFilter> filters,
                              PathView initialPath, const Window* parentWindow,
                              StringView dialogTitle)
    {
        Vector<String>            filterStorage;
        Vector<nfdu8filteritem_t> nfdFilters;
        ProcessFilters(filters, filterStorage, nfdFilters);

        Path               defaultPath  = initialPath;
        nfdwindowhandle_t  parentHandle = GetNativeHandle(parentWindow);

        NFD::UniquePathSet outPaths;
        nfdresult_t        result = NFD::OpenDialogMultiple(
            outPaths, nfdFilters.Empty() ? nullptr : nfdFilters.Raw(),
            static_cast<nfdfiltersize_t>(nfdFilters.Size()),
            !defaultPath.Empty() ? defaultPath.Raw() : nullptr, parentHandle);

        Vector<Path> selectedPaths;
        if (result == NFD_OKAY)
        {
            nfdpathsetsize_t count = 0;
            NFD::PathSet::Count(outPaths, count);
            selectedPaths.Reserve(static_cast<usize>(count));

            for (nfdpathsetsize_t i = 0; i < count; ++i)
            {
                NFD::UniquePathSetPath path;
                NFD::PathSet::GetPath(outPaths, i, path);
                selectedPaths.EmplaceBack(path.get());
            }
        }

        return selectedPaths;
    }
    Optional<Path> Window::OpenFolder(PathView      initialPath,
                                      const Window* parentWindow,
                                      StringView    dialogTitle)
    {
        NFD::Guard        nfdGuard;

        Path              defaultPath  = initialPath;
        nfdwindowhandle_t parentHandle = GetNativeHandle(parentWindow);

        NFD::UniquePath   outPath;
        nfdresult_t       result
            = NFD::PickFolder(outPath, defaultPath.Raw(), parentHandle);

        if (result == NFD_OKAY) return Path(outPath.get());
        return NullOpt;
    }
    Optional<Path> Window::SaveFile(Span<const Window::FileFilter> filters,
                                    PathView                       initialPath,
                                    const Window*                  parentWindow,
                                    StringView                     dialogTitle)
    {
        Vector<String>            filterStorage;
        Vector<nfdu8filteritem_t> nfdFilters;
        ProcessFilters(filters, filterStorage, nfdFilters);

        Path              defaultPath  = initialPath;
        String            titleStr     = dialogTitle;
        nfdwindowhandle_t parentHandle = GetNativeHandle(parentWindow);

        NFD::UniquePath   outPath;
        nfdresult_t       result = NFD::SaveDialog(
            outPath, nfdFilters.Empty() ? nullptr : nfdFilters.Raw(),
            static_cast<nfdfiltersize_t>(nfdFilters.Size()),
            !defaultPath.Empty() ? defaultPath.Raw() : nullptr,
            !titleStr.Empty() ? titleStr.Raw() : nullptr, parentHandle);

        return (result == NFD_OKAY) ? Optional<Path>(Path(outPath.get()))
                                    : NullOpt;
    }
#else
    Optional<Path> Window::OpenFile(Span<const Window::FileFilter> filters,
                                    PathView                       initialPath,
                                    const Window*                  parentWindow,
                                    StringView                     dialogTitle)
    {
        return NullOpt;
    }
    Vector<Path>
    Window::OpenMultipleFiles(Span<const Window::FileFilter> filters,
                              PathView initialPath, const Window* parentWindow,
                              StringView dialogTitle)
    {
        return {};
    }
    Optional<Path> Window::OpenFolder(PathView      initialPath,
                                      const Window* parentWindow,
                                      StringView    dialogTitle)
    {
        return NullOpt;
    }
    Optional<Path> Window::SaveFile(Span<const Window::FileFilter> filters,
                                    PathView                       initialPath,
                                    const Window*                  parentWindow,
                                    StringView                     dialogTitle)
    {
        return NullOpt;
    }

#endif

    Scope<Window> Window::Create(const WindowSpecification& specs)
    {
        return Scope<Window>(s_CreateWindow(specs));
    }

    Window* Window::CreateForImGui(const WindowSpecification& specs)
    {
        const_cast<WindowSpecification&>(specs).NoAPI = true;

        return s_CreateWindow(specs);
    }

    WindowSubsystem Window::WindowSubsystem()
    {
        static enum WindowSubsystem subsystem = []() -> enum WindowSubsystem
        {
            enum WindowSubsystem subsystem = ChooseSubsystem();

            VtCoreInfo("Window: Using {}...",
                       StringUtils::ToString(subsystem).Raw() + 1);
            return subsystem;
        }
        ();

        return subsystem;
    };
}; // namespace Vortex
