//
// Created by Vitriol1744 on 06.08.2021.
//
#pragma once

#include "Vortex/Audio/OpenAL.hpp"
#include "Vortex/Core/Core.hpp"

namespace Vortex::Audio
{
    class VT_API AudioListener
    {
        public:
            static ALvoid SetGain(ALfloat gain);
            inline static ALvoid SetPosition(Math::Vec3 position) { SetPosition(position.x, position.y, position.z); }
            static ALvoid SetPosition(ALfloat x, ALfloat y, ALfloat z);
            inline static ALvoid SetVelocity(Math::Vec3 velocity) { SetVelocity(velocity.x, velocity.y, velocity.z); }
            static ALvoid SetVelocity(ALfloat x, ALfloat y, ALfloat z);
            static ALvoid SetOrientation(Math::Vec3 at, Math::Vec3 up);
    };
}

