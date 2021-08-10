//
// Created by Vitriol1744 on 06.08.2021.
//
#include "AudioListener.hpp"

#include "AL/al.h"

namespace Vortex::Audio
{
    ALvoid AudioListener::SetGain(ALfloat gain)
    {
        alCall(alListenerf(AL_GAIN, gain));
    }
    ALvoid AudioListener::SetPosition(ALfloat x, ALfloat y, ALfloat z)
    {
        alCall(alListener3f(AL_POSITION, x, y, z));
    }
    ALvoid AudioListener::SetVelocity(ALfloat x, ALfloat y, ALfloat z)
    {
        alCall(alListener3f(AL_VELOCITY, x, y, z));
    }
    ALvoid AudioListener::SetOrientation(Math::Vec3 at, Math::Vec3 up)
    {
        ALfloat orientation[] = { at.x, at.y, at.z, up.x, up.y, up.z };
        alCall(alListenerfv(AL_ORIENTATION, orientation));
    }
}