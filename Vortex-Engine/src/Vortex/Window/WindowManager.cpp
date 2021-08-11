//
// Created by Vitriol1744 on 23.06.2021.
//
#include "vtpch.hpp"
#include "WindowManager.hpp"

#include "Vortex/Graphics/API/OpenGL46/OpenGL.hpp"

#undef CreateWindow

namespace Vortex
{
    uint32 count = 0;
    uint32 vao;

    Ref<IWindow> WindowManager::NewWindow(int32 width, int32 height, std::string_view title, Ref<IWindow> share)
    {
        windows.push_back(IWindow::CreateWindow(width, height, title, share));

        count++;

        return windows.back();
    }
}