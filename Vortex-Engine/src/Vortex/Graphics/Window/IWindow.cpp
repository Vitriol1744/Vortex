//
// Created by Vitriol1744 on 07.09.2022.
//
#include "Vortex/vtpch.hpp"
#include "IWindow.hpp"

#ifdef VT_PLATFORM_WINDOWS
#include "Vortex/Graphics/Window/Win32/Win32WindowImpl.hpp"
#endif
#undef CreateWindow

namespace Vortex
{
    void IWindow::PollEvents()
    {
        WindowImpl::PollEvents();
    }

    Ref<IWindow> IWindow::CreateWindow(uint32 width, uint32 height, std::string title)
    {
        return CreateWindow(width, height, 32, title);
    }
    Ref<IWindow> IWindow::CreateWindow(uint32 width, uint32 height, uint32 bpp, std::string title)
    {
        return CreateRef<WindowImpl>(width, height, bpp, title);
    }
}