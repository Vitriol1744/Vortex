//
// Created by Vitriol1744 on 31.06.2021.
//
#pragma once

#include "Vortex/Core/Core.hpp"

#include "Vortex/Audio/ISoundBuffer.hpp"

#include <xaudio2.h>


namespace Vortex::Audio
{
	class VT_API XA2SoundBuffer : public ISoundBuffer
	{
		public:
			XA2SoundBuffer(std::string_view filepath);
			~XA2SoundBuffer();

			inline XAUDIO2_BUFFER GetBuffer() noexcept { return buffer; }
			inline WAVEFORMATEXTENSIBLE GetFormat() noexcept { return format; }

		private:
			XAUDIO2_BUFFER buffer;
			WAVEFORMATEXTENSIBLE format;
	};
}