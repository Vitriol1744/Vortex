//
// Created by Vitriol1744 on 03.08.2021.
//
#include "SoundBuffer.hpp"

#include "Vortex/Utility/AudioLoader.hpp"

#include "AL/al.h"

namespace Vortex::Audio
{
	using namespace Utility;

	SoundBuffer::~SoundBuffer()
	{
	    alCall(alDeleteBuffers(1, &id));
		if (data) delete data;
	}

	ALvoid SoundBuffer::Initialize()
	{
	    alCall(alGenBuffers(1, &id));
	}

	ALvoid SoundBuffer::LoadFromFile(std::string_view filepath)
	{
		ALuint size = 0;
		WAVFormat format = {};

		uint8* data = AudioLoader::LoadWAV(filepath, size, format);
		Load(data, size, format);
	}
	ALvoid SoundBuffer::Load(uint8* data, ALuint size, WAVFormat format)
	{
	    //TODO: Temporary! Remove Later!
        ALenum alFormat = {};
        if (format.channelsCount == 1 && format.bitsPerSample == 8) alFormat = AL_FORMAT_MONO8;
        else if (format.channelsCount == 1 && format.bitsPerSample == 16) alFormat = AL_FORMAT_MONO16;
        else if (format.channelsCount == 2 && format.bitsPerSample == 8) alFormat = AL_FORMAT_STEREO8;
        else if (format.channelsCount == 2 && format.bitsPerSample == 16) alFormat = AL_FORMAT_STEREO16;
        else VTCoreLogWarn("ERROR: unrecognised wave format!, channels: {}, bps: {}", format.channelsCount, format.bitsPerSample);

        alCall(alBufferData(id, alFormat, data, size, format.sampleRate));
	}

	ALint SoundBuffer::Frequency() const
	{
        ALint result;
        alGetBufferi(id, AL_FREQUENCY, &result);

        return result;
	}
	ALint SoundBuffer::DepthBits() const
	{
        ALint result;
        alGetBufferi(id, AL_BITS, &result);

        return result;
	}
	ALint SoundBuffer::ChannelsCount() const
	{
        ALint result;
        alGetBufferi(id, AL_CHANNELS, &result);

        return result;
	}
	ALint SoundBuffer::Size() const
	{
        ALint result;
        alGetBufferi(id, AL_SIZE, &result);

        return result;
	}
	ALchar* SoundBuffer::Data() const
	{
	    return data;
	}
}