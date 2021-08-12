//
// Created by Vitriol1744 on 10.08.2021.
//
#include "Engine.hpp"

#include "Vortex/Core/Time.hpp"
#include "Vortex/Core/EventSystem.hpp"
#include "Vortex/Audio/AudioManager.hpp"
//TODO: Remove later!
#include "Vortex/Graphics/API/OpenGL46/OpenGL.hpp"
#include "Vortex/Utility/AudioLoader.hpp"
#include "Vortex/Input/Gamepad.hpp"

#include <iostream>

namespace Vortex
{
    using namespace Audio;

    Engine* Engine::instance = nullptr;

    Engine::Engine(std::vector<char*>& arguments)
    {
        VT_CORE_ASSERT_MSG(instance == nullptr, "Engine can only have 1 instance!");
        instance = this;
        this->arguments = arguments;
    }

    void Engine::Initialize()
    {
        //NOTE: Logger should always be initialized first!
        LoggingManager::Instance(); // Initialize Logger!
        Platform::Internal::Initialize();
        Time::Instance(); // Initialize Time!
        AudioManager::Initialize();
        VTCoreLogInfo("");
        app = CreateApplication(arguments);
        VTCoreLogInfo("Arguments: ");
        for (int i = 0; i < arguments.size(); i++) VTCoreLogInfo("Arg[{}]: {}", i, arguments[i]);
        VTCoreLogInfo("");
    }
    void Engine::Shutdown()
    {
        AudioManager::Shutdown();
        Platform::Internal::Shutdown();
    }

    void Engine::Run()
    {
        double previousFrame = Time::GetTime().Seconds();
        double fpsTimer = previousFrame;

        //TODO: Remove Later!
        //glEnable(GL_DEPTH_TEST);
        //glBlendFunc(GL_SRC_ALPHA, GL_SRC_ALPHA);

        app->Initialize();
        #pragma clang diagnostic push
        #pragma ide diagnostic ignored "EndlessLoop"
        #pragma ide diagnostic ignored "ConstantConditionsOC"
        while (running)
        {
            //Gamepad::PollInput();
            if (Time::GetTime().Seconds() - fpsTimer >= 1.0f)
            {
                Time::SetFPSCounter(frames);
                frames = 0;
                fpsTimer = Time::GetTime().Seconds();
            }

            Time::SetDeltaTime(Time::GetTime().Seconds() - previousFrame);
            previousFrame = Time::GetTime().Seconds();

            app->Update();
            app->Render();

            if (framerateLimit > 0) Time::Sleep(Timestep(1.0 / framerateLimit - (Time::GetTime().Seconds() - previousFrame)));

            frames++;
            EventSystem::Instance()->PollEvents();
        }
        #pragma clang diagnostic pop
        app->Shutdown();
    }
}
