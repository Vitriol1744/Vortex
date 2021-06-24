//
// Created by Vitriol1744 on 22.06.2021.
//
#include "Application.hpp"
#include "Window/Win32/Win32Window.hpp"

namespace Vortex
{
    Application::Application()
    {
        Logger::Initialize();
        window = WindowManager::Get()->NewWindow(800, 600, L"Vortex Engine!");
    }

    void Application::Run()
    {
        while (running)
        {
            running = window->IsOpen();
            window->Update();
        }
    }
}