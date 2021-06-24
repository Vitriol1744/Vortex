//
// Created by Vitriol1744 on 23.06.2021.
//
#include "WindowManager.hpp"

namespace Vortex
{
    Ref<IWindow> WindowManager::NewWindow(int32 width, int32 height, std::wstring_view title)
    {
        windows.push_back(IWindow::CreateWindow(width, height, title));

        return windows.back();
    }
}