//
// Created by Vitriol1744 on 03.08.2021.
//
#include "SoundBuffer.hpp"

#include "Vortex/Utility/AudioLoader.hpp"

#include "AL/al.h"

namespace Vortex::Audio
{
	using namespace Utility;

	SoundBuffer::SoundBuffer()
	{
		alGenBuffers(1, &id);
	}

	SoundBuffer::~SoundBuffer()
	{
		alDeleteBuffers(1, &id);
		if (data) delete data;
	}

	ALvoid SoundBuffer::LoadFromFile(std::string_view filepath)
	{
		uint32 size = 0;
		WAVFormat format = {};

		uint8* data = AudioLoader::LoadWAV(filepath, size, format);
		Load(data);
	}

	ALvoid SoundBuffer::Load(uint8* data)
	{

	}
}