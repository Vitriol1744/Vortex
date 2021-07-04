//
// Created by Vitriol1744 on 29.06.2021.
//
#include "vtpch.hpp"
#include "Core/Macros.hpp"

#include "Graphics/OpenGL46/GL46Context.hpp"
#include "Graphics/IRendererAPI.hpp"

#include <cstdio>

#ifdef VT_PLATFORM_LINUX
#include "PosixWindow.hpp"
namespace Vortex
{
    uint32   WindowImpl::windowsCount = 0;
    Display* WindowImpl::display = nullptr;

    WindowImpl::WindowImpl(int32 width, int32 height, std::wstring_view title)
    {
        if (windowsCount == 0) Initialize();

        window = XCreateSimpleWindow
        (
            display, DefaultRootWindow(display),
            10, 10, 800, 600, 0, BlackPixel(display, DefaultScreen(display)),
            BlackPixel(display, DefaultScreen(display))
        );
        windowsCount++;

        IRendererAPI::Initialize();
        switch (Vortex::IRendererAPI::Get()->GetGraphicsAPI())
        {
            case GraphicsAPI::OpenGL46:
            {
                context = CreateScope<GL46Context>(reinterpret_cast<void*>(&window));
                break;
            }
            case GraphicsAPI::None:

            default:
                VT_CORE_LOG_FATAL("Graphics API Not Supported!");
                break;
        }
        
        VT_CORE_LOG_TRACE("Graphics Context Created!");
        XSelectInput
        (
            display, window, StructureNotifyMask | KeyPressMask | KeyReleaseMask |
            PointerMotionMask | ButtonPressMask | ButtonReleaseMask | ExposureMask |
            FocusChangeMask | VisibilityChangeMask | EnterWindowMask | LeaveWindowMask | PropertyChangeMask
        );
        XMapWindow(display, window);
    }

    WindowImpl::~WindowImpl()
    {
        XDestroyWindow(display, window);
        windowsCount--;
        if (windowsCount == 0) Shutdown();
    }

    void WindowImpl::Update()
    {
        while (XEventsQueued(display, QueuedAlready))
        {
            XEvent event;
            XNextEvent(display, &event);
        }
    }

    void WindowImpl::Present()
    {
        context->Present();
    }

    void WindowImpl::SetTitle(std::wstring_view title)
    {
        std::string title_(title.begin(), title.end());
        XStoreName(display, window, title_.data());
    }

    void WindowImpl::ActivateContext()
    {

    }

    void WindowImpl::Initialize()
    {
        display = XOpenDisplay(nullptr);
        VT_CORE_ASSERT(display != nullptr);
    }

    void WindowImpl::Shutdown()
    {

    }
}
#endif
