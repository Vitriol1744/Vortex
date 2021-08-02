//
// Created by Vitriol1744 on 31.06.2021.
//
#include "XA2SoundBuffer.hpp"

#include "Vortex/Utility/AudioLoader.hpp"

using namespace Vortex::Utility;


namespace Vortex::Audio
{
	XA2SoundBuffer::XA2SoundBuffer(std::string_view filepath)
	{
		uint32 size;
		uint8* data = AudioLoader::LoadWAV(filepath, size, *(reinterpret_cast<Utility::WAVFormat*>(&format)));
		
		memset(&buffer, 0, sizeof(buffer));
		buffer.AudioBytes = size;
		buffer.pAudioData = data;
		buffer.Flags = XAUDIO2_END_OF_STREAM;
	}

	XA2SoundBuffer::~XA2SoundBuffer()
	{
		delete buffer.pAudioData;
	}
}