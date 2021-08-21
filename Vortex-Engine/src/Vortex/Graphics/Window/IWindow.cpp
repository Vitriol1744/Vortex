//
// Created by Vitriol1744 on 22.06.2021.
//
#include "vtpch.hpp"
#include "IWindow.hpp"

#ifdef VT_PLATFORM_WINDOWS
    #include "Vortex/Window/Win32/Win32Window.hpp"
    #undef CreateWindow
#elif defined(VT_PLATFORM_LINUX)
    #include "Graphics/Window/Posix/X11Window.hpp"
#endif

namespace Vortex
{
    Ref<IWindow> IWindow::CreateWindow(uint32 width, uint32 height, std::string_view title, Ref<IWindow> share)
    {
        return CreateWindow(width, height, 32, title, share);
    }
    Ref<IWindow> IWindow::CreateWindow(uint32 width, uint32 height, uint32 bitsPerPixel, std::string_view title, Ref<IWindow> share)
    {
        return CreateRef<WindowImpl>(width, height, bitsPerPixel, title, share);
    }
}