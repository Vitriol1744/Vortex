//
// Created by Vitriol1744 on 22.06.2021.
//
#include "Application.hpp"
#include "Window/Win32/Win32Window.hpp"

#include "Graphics/OpenGL46/OpenGL.hpp"

namespace Vortex
{
    Application::Application()
    {
        Logger::Initialize(); // <- This Function has to be called first!

        window = WindowManager::Get()->NewWindow(800, 600, L"Vortex Engine!");
    }

    void Application::Run()
    {
        glClearColor(0.0f, 1.0f, 1.0f, 1.0f);
        while (running)
        {
            running = window->IsOpen();

            window->Update();
            window->SwapBuffers();
        }
    }
}