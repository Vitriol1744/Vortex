//
// Created by Vitriol1744 on 08.09.2022.
//
#include "Vortex/vtpch.hpp"
#include "WindowManager.hpp"

namespace Vortex
{
    Ref<IWindow> WindowManager::NewWindow(int32 width, int32 height, std::string_view title)
    {
        windows.push_back(IWindow::CreateWindow(width, height, title.data()));

        return windows.back();
    }
}