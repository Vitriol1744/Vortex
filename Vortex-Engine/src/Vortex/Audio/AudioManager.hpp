//
// Created by Vitriol1744 on 06.08.2021.
//
#pragma once

#include "Vortex/Audio/OpenAL.hpp"
#include "Core/Core.hpp"

#include <vector>
#include <string>

namespace Vortex::Audio
{
    using AudioDevice = ALCuint;

    class VT_API AudioManager
    {
        public:
            static ALCvoid Initialize();
            static ALCvoid Shutdown();

            VT_NODISCARD static std::vector<std::string> GetAudioDevices();
            VT_NODISCARD static ALCuint AddAudioDevice(std::string_view name);
            static ALCvoid RemoveAudioDevice(std::string_view name);
            static ALCvoid RemoveAudioDevice(AudioDevice device);

            static ALCvoid SetAudioDevice(std::string_view name);
            static ALCvoid SetAudioDevice(AudioDevice device);
    };
}