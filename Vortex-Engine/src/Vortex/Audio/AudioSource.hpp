//
// Created by Vitriol1744 on 04.08.2021.
//
#pragma once

#include "Vortex/Audio/OpenAL.hpp"
#include "Vortex/Audio/SoundBuffer.hpp"

#include "Vortex/Core/Core.hpp"
#include "Vortex/Core/Timestep.hpp"

namespace Vortex
{
    namespace Audio
    {
        enum class SourceType
                {
                Undetermined,
                Static,
                Streaming
                };

        class AudioSource : public NonCopyable<AudioSource>
        {
            public:
                inline AudioSource(bool initialize = false) { if (initialize) Initialize(); }
                inline AudioSource(SoundBuffer& buffer) : AudioSource(true) { SetBuffer(buffer); }
                ~AudioSource();

                ALvoid Initialize();

                ALvoid Play() const;
                ALvoid Stop() const;

                ALboolean IsPlaying() const;

                ALvoid SetPitch(ALfloat pitch) const;
                ALvoid SetGain(ALfloat gain) const;
                ALvoid SetMaxDistance(ALfloat distance) const;
                ALvoid SetRollOffFactor(ALfloat factor) const;
                ALvoid SetReferenceDistance(ALfloat distance) const;
                ALvoid SetMinGain(ALfloat min) const;
                ALvoid SetMaxGain(ALfloat max) const;
                ALvoid SetConeOuterGain(ALfloat gain) const;
                ALvoid SetConeInnerAngle(ALfloat angle) const;
                ALvoid SetConeOuterAngle(ALfloat angle) const;
                inline ALvoid SetPosition(Math::Vec3 position) const { SetPosition(position.x, position.y, position.z); }
                ALvoid SetPosition(ALfloat x, ALfloat y, ALfloat z) const;
                inline ALvoid SetVelocity(Math::Vec3 velocity) const { SetVelocity(velocity.x, velocity.y, velocity.z); }
                ALvoid SetVelocity(ALfloat x, ALfloat y, ALfloat z) const;
                inline ALvoid SetDirection(Math::Vec3 direction) const { SetDirection(direction.x, direction.y, direction.z); }
                ALvoid SetDirection(ALfloat x, ALfloat y, ALfloat z) const;
                ALvoid SetListenerRelative(ALboolean relative) const;
                ALvoid SetType(SourceType type) const;
                ALvoid SetLooping(ALboolean looping = true) const;
                ALvoid SetBuffer(SoundBuffer& buffer) const;
                ALvoid SetOffset(Timestep offset) const;
                ALvoid SetSamplesOffset(ALfloat offset) const;
                ALvoid SetBytesOffset(ALfloat offset) const;

            private:
                ALuint id;
        };
    }
    template class VT_API NonCopyable<Audio::AudioSource>;
}
