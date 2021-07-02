//
// Created by Vitriol1744 on 22.06.2021.
//
#include "vtpch.hpp"
#include "Application.hpp"

#include "Graphics/OpenGL46/OpenGL.hpp"
#include "Core/Time.hpp"

#include <iostream>

namespace Vortex
{
    Application::Application()
    {
        Logger::Initialize(); // <- This Function has to be called first!
        Time::Get();
    }

    void Application::Run()
    {
        double previousFrame = Time::GetTimeInSeconds();
        double fpsTimer = previousFrame;
        while (running)
        {
            if (Time::GetTimeInSeconds() - fpsTimer >= 1.0f)
            {
                Time::Get()->SetFPSCounter(frames);;
                frames = 0;
                fpsTimer = Time::GetTimeInSeconds();
            }

            Time::Get()->SetDeltaTime(Time::GetTimeInSeconds() - previousFrame);
            previousFrame = Time::GetTimeInSeconds();

            Update();
            Render();
            if (framerateLimit > 0)
                while ((Time::GetTimeInSeconds() - previousFrame) < (1.0f / (double)framerateLimit));
            frames++;
        }
    }
}