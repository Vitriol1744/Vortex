//
// Created by Vitriol1744 on 31.06.2021.
//
#pragma once

#include "Vortex/Core/Core.hpp"

#include "Vortex/Audio/ISound.hpp"
#include "Vortex/Audio/XAudio2/XA2SoundBuffer.hpp"

#include <xaudio2.h>

namespace Vortex::Audio
{
	class VT_API XA2Sound : public ISound
	{
		public:
			XA2Sound(Ref<ISoundBuffer> buffer);
			~XA2Sound();

			virtual void Play() override;

		private:
			Ref<XA2SoundBuffer> buffer;
			IXAudio2SourceVoice* sourceVoice;
	};
}