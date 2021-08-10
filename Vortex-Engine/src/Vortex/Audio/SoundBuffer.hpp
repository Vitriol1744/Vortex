//
// Created by Vitriol1744 on 03.08.2021.
//
#pragma once

#include "Vortex/Core/Core.hpp"

#include "Vortex/Audio/OpenAL.hpp"
#include "Vortex/Utility/AudioLoader.hpp"

#include <string_view>

namespace Vortex::Audio
{
	class VT_API SoundBuffer
	{
		public:
			inline explicit SoundBuffer(bool initialize = false) { if (initialize) Initialize(); }
			inline explicit SoundBuffer(const char* filepath) : SoundBuffer(true) { LoadFromFile(filepath); }
			inline SoundBuffer(uint8* data, ALuint size, Utility::WAVFormat format) : SoundBuffer(true) { Load(data, size, format); }
			~SoundBuffer();

			ALvoid Initialize();

			ALvoid LoadFromFile(std::string_view filepath);
			ALvoid Load(uint8* data, ALuint size, Utility::WAVFormat format);

			ALint Frequency() const;
			ALint DepthBits() const;
			ALint ChannelsCount() const;
			ALint Size() const;
			ALchar* Data() const;
			inline ALuint ID() const { return id; }

		private:
            ALuint id;
			ALchar* data;
	};
}