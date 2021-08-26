//
// Created by Vitriol1744 on 04.08.2021.
//
#include "AudioSource.hpp"

#include "AL/al.h"

namespace Vortex::Audio
{
    AudioSource::~AudioSource()
    {
        alCall(alDeleteSources(1, &id));
    }

    ALvoid AudioSource::Initialize()
    {
        alCall(alGenSources(1, &id));
    }

    ALvoid AudioSource::Play() const
    {
        alCall(alSourcePlay(id));
    }
    ALvoid AudioSource::Stop() const
    {
        alCall(alSourceStop(id));
    }

    ALboolean AudioSource::IsPlaying() const
    {
        ALint state;
        alCall(alGetSourcei(id, AL_SOURCE_STATE, &state));

        return state == AL_PLAYING;
    }

    ALvoid AudioSource::SetPitch(ALfloat pitch) const
    {
        alCall(alSourcef(id, AL_PITCH, pitch));
    }
    ALvoid AudioSource::SetGain(ALfloat gain) const
    {
        alCall(alSourcef(id, AL_GAIN, gain));
    }
    ALvoid AudioSource::SetMaxDistance(ALfloat distance) const
    {
        alCall(alSourcef(id, AL_MAX_DISTANCE, distance));
    }
    ALvoid AudioSource::SetRollOffFactor(ALfloat factor) const
    {
        alCall(alSourcef(id, AL_ROLLOFF_FACTOR, factor));
    }
    ALvoid AudioSource::SetReferenceDistance(ALfloat distance) const
    {
        alCall(alSourcef(id, AL_REFERENCE_DISTANCE, distance));
    }
    ALvoid AudioSource::SetMinGain(ALfloat min) const
    {
        alCall(alSourcef(id, AL_MIN_GAIN, min));
    }
    ALvoid AudioSource::SetMaxGain(ALfloat max) const
    {
        alCall(alSourcef(id, AL_MAX_GAIN, max));
    }
    ALvoid AudioSource::SetConeOuterGain(ALfloat gain) const
    {
        alCall(alSourcef(id, AL_CONE_OUTER_GAIN, gain));
    }
    ALvoid AudioSource::SetConeInnerAngle(ALfloat angle) const
    {
        alCall(alSourcef(id, AL_CONE_INNER_ANGLE, angle));
    }
    ALvoid AudioSource::SetConeOuterAngle(ALfloat angle) const
    {
        alCall(alSourcef(id, AL_CONE_OUTER_ANGLE, angle));
    }
    ALvoid AudioSource::SetPosition(ALfloat x, ALfloat y, ALfloat z) const
    {
        alCall(alSource3f(id, AL_POSITION, x, y, z));
    }
    ALvoid AudioSource::SetVelocity(ALfloat x, ALfloat y, ALfloat z) const
    {
        alCall(alSource3f(id, AL_VELOCITY, x, y, z));
    }
    ALvoid AudioSource::SetDirection(ALfloat x, ALfloat y, ALfloat z) const
    {
        alCall(alSource3f(id, AL_DIRECTION, x, y, z));
    }
    ALvoid AudioSource::SetListenerRelative(ALboolean relative) const
    {
        alCall(alSourcei(id, AL_SOURCE_RELATIVE, relative));
    }
    ALvoid AudioSource::SetType(SourceType type) const
    {
        ALenum _type;
        switch (type)
        {
            case SourceType::Undetermined:
                _type = AL_UNDETERMINED;
                break;
            case SourceType::Static:
                _type = AL_STATIC;
                break;
            case SourceType::Streaming:
                _type = AL_STREAMING;
                break;
        }

        alCall(alSourcei(id, AL_SOURCE_TYPE, _type));
    }
    ALvoid AudioSource::SetLooping(ALboolean looping) const
    {
        alCall(alSourcei(id, AL_LOOPING, looping));
    }
    ALvoid AudioSource::SetBuffer(SoundBuffer& buffer) const
    {
        alCall(alSourcei(id, AL_BUFFER, buffer.ID()));
    }
    ALvoid AudioSource::SetOffset(Timestep offset) const
    {
        alCall(alSourcef(id, AL_SEC_OFFSET, offset.Seconds()));
    }
    ALvoid AudioSource::SetSamplesOffset(ALfloat offset) const
    {
        alCall(alSourcef(id, AL_SAMPLE_OFFSET, offset));
    }
    ALvoid AudioSource::SetBytesOffset(ALfloat offset) const
    {
        alCall(alSourcef(id, AL_BYTE_OFFSET, offset));
    }
}
