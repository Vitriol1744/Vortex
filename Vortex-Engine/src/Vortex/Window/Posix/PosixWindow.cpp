//
// Created by Vitriol1744 on 29.06.2021.
//

#ifdef VT_PLATFORM_LINUX
#include "PosixWindow.hpp"
namespace Vortex
{
    WindowImpl::WindowImpl(int32 width, int32 height, std::wstring_view title)
    {
        display = XOpenDisplay(NULL);
        VT_CORE_ASSERT(display != nullptr);
    }

    WindowImpl::~WindowImpl()
    {

    }

    void WindowImpl::Update()
    {

    }

    void WindowImpl::Present()
    {

    }

    void WindowImpl::SetTitle(std::wstring_view title)
    {

    }

    void WindowImpl::ActivateContext()
    {

    }
}
#endif
