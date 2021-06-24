//
// Created by Vitriol1744 on 22.06.2021.
//
#include "IWindow.hpp"

#ifdef _WIN32
#include "Win32/Win32Window.hpp"
#undef CreateWindow
#endif

namespace Vortex
{
    Ref<IWindow> IWindow::CreateWindow(int width, int height, std::wstring_view title)
    {
        return CreateRef<WindowImpl>(width, height, title);
    }
}