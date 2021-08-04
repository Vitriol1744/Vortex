//
// Created by Vitriol1744 on 03.08.2021.
//
#pragma once

#include "Vortex/Core/Core.hpp"

#include <string_view>

namespace Vortex::Audio
{
	class VT_API SoundBuffer
	{
		public:
			SoundBuffer();
			inline SoundBuffer(std::string_view filepath) : SoundBuffer() { LoadFromFile(filepath); }
			inline SoundBuffer(uint8* data) : SoundBuffer() { Load(data); }
			~SoundBuffer();

			void LoadFromFile(std::string_view filepath);
			void Load(uint8* data);

		private:
			uint32 id;
			uint8* data;
	};
}