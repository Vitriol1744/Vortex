//
// Created by Vitriol1744 on 22.06.2021.
//
#include "Application.hpp"

#include "Core/Time.hpp"
#include "Core/EventSystem.hpp"

//TODO: Remove later!
#include "Graphics/API/OpenGL46/OpenGL.hpp"

using namespace Vortex;

#undef None

namespace Vortex
{
    Application::Application()
    {
        LoggingManager::Instance(); // Initialize Logger!
        Time::Instance(); // Initialize Time!
    }

    void Application::Run()
    {
        double previousFrame = Time::GetTime().Seconds();
        double fpsTimer = previousFrame;

        //TODO: Remove Later!
        glEnable(GL_DEPTH_TEST);
        glBlendFunc(GL_SRC_ALPHA, GL_SRC_ALPHA);

        while (running)
        {
            if (Time::GetTime().Seconds() - fpsTimer >= 1.0f)
            {
                Time::SetFPSCounter(frames);
                frames = 0;
                fpsTimer = Time::GetTime().Seconds();
            }

            Time::SetDeltaTime(Time::GetTime().Seconds() - previousFrame);
            previousFrame = Time::GetTime().Seconds();

            Update();
            Render();

            if (framerateLimit > 0) Time::Sleep(Timestep(1.0 / framerateLimit - (Time::GetTime().Seconds() - previousFrame)));

            frames++;
            EventSystem::Instance()->Update();
        }
    }
}