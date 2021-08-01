//
// Created by Vitriol1744 on 31.06.2021.
//
#pragma once

#include "Vortex/Core/Core.hpp"

#include "Vortex/Audio/IAudioContext.hpp"

#include <xaudio2.h>
#include <wrl/client.h>

namespace Vortex::Audio
{
	class VT_API XA2AudioContext : public IAudioContext
	{
		public:
			XA2AudioContext();

			inline Microsoft::WRL::ComPtr<IXAudio2> GetAudioEngine() noexcept { return xAudio2; }

		private:
			Microsoft::WRL::ComPtr<IXAudio2> xAudio2 = nullptr;
			IXAudio2MasteringVoice* masterVoice = nullptr;
	};
}