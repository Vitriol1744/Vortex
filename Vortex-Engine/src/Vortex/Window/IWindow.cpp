//
// Created by Vitriol1744 on 22.06.2021.
//
#include "vtpch.hpp"
#include "IWindow.hpp"

#ifdef VT_PLATFORM_WINDOWS
    #include "Win32/Win32Window.hpp"
    #undef CreateWindow
#elif defined(VT_PLATFORM_LINUX)
    #include "Posix/PosixWindow.hpp"
#endif

namespace Vortex
{
    Ref<IWindow> IWindow::CreateWindow(int width, int height, std::wstring_view title)
    {
        return CreateRef<WindowImpl>(width, height, title);
    }
}