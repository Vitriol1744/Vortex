//
// Created by Vitriol1744 on 22.06.2021.
//
#include "Application.hpp"

#include "Vortex/Core/Time.hpp"
#include "Vortex/Core/EventSystem.hpp"

//TODO: Remove later!
#include "Vortex/Graphics/API/OpenGL46/OpenGL.hpp"
#include "Vortex/Utility/AudioLoader.hpp"
#include "Vortex/Audio/AudioManager.hpp"
#include "Vortex/Audio/ISound.hpp"
//^^^^^^^^^^^^^^^^^^^|
//===================|

using namespace Vortex;

#undef None

namespace Vortex
{
    using namespace Audio;

    Application::Application()
    {
        //NOTE: Logger should always be initialized first!
        LoggingManager::Instance(); // Initialize Logger!
        Platform::Initialize();
        Time::Instance(); // Initialize Time!
        AudioManager::Initialize();

        static Ref<ISound> sound1 = ISound::Create("assets/sounds/breakout.wav");
        static Ref<ISound> sound2 = ISound::Create("assets/sounds/sound.wav");
        sound1->Play();
        //sound2->Play();
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