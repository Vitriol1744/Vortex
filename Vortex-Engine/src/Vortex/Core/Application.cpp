//
// Created by Vitriol1744 on 22.06.2021.
//
#include "Application.hpp"

#include "Vortex/Core/Time.hpp"
#include "Vortex/Core/EventSystem.hpp"

//TODO: Remove later!
#include "Vortex/Graphics/API/OpenGL46/OpenGL.hpp"
#include "Vortex/Utility/AudioLoader.hpp"
#include "AL/al.h"
#include "AL/alc.h"
#include "Vortex/Input/Gamepad.hpp"
//^^^^^^^^^^^^^^^^^^^|
//===================|

using namespace Vortex;
using namespace Vortex::Input;
using namespace Vortex::Utility;

#undef None

ALCdevice* device;
ALCcontext* context;
ALuint buffer;
ALuint source;
uint8* data;
WAVFormat format;
uint32 size;

namespace Vortex
{
    Application::Application()
    {
        //NOTE: Logger should always be initialized first!
        LoggingManager::Instance(); // Initialize Logger!
        Platform::Initialize();
        Time::Instance(); // Initialize Time!
        
        device = alcOpenDevice(nullptr);
        context = alcCreateContext(device, nullptr);

        if (!alcMakeContextCurrent(context))
        {
            VT_CORE_LOG_ERROR("Failed to make context current!");
        }

        alGenBuffers(1, &buffer);
        data = AudioLoader::LoadWAV("assets/sounds/breakout.wav", size, format);

        ALenum alFormat = {};
        if (format.channelsCount == 1 && format.bitsPerSample == 8)
            alFormat = AL_FORMAT_MONO8;
        else if (format.channelsCount == 1 && format.bitsPerSample == 16)
            alFormat = AL_FORMAT_MONO16;
        else if (format.channelsCount == 2 && format.bitsPerSample == 8)
            alFormat = AL_FORMAT_STEREO8;
        else if (format.channelsCount == 2 && format.bitsPerSample == 16)
            alFormat = AL_FORMAT_STEREO16;
        else
        {
            //VT_CORE_LOG_ERROR("FormatType: {}, channelsCount");
            VT_CORE_LOG_ERROR("ERROR: unrecognised wave format!, channels: {}, bps: {}", format.channelsCount, format.bitsPerSample);
        }

        alBufferData(buffer, alFormat, data, size, format.sampleRate);
        alGenSources(1, &source);
        alSourcef(source, AL_PITCH, 1);
        alSourcef(source, AL_GAIN, 1.0f);
        alSource3f(source, AL_POSITION, 0.0f, 0.0f, 0.0f);
        alSource3f(source, AL_VELOCITY, 0, 0, 0);
        alSourcei(source, AL_LOOPING, AL_FALSE);
        alSourcei(source, AL_BUFFER, buffer);

        alSourcePlay(source);
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
            Gamepad::PollInput();
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