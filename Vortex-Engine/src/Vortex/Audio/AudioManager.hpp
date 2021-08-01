//
// Created by Vitriol1744 on 31.06.2021.
//
#pragma once

#include "Vortex/Core/Core.hpp"

#include "Vortex/Audio/IAudioContext.hpp"

namespace Vortex::Audio
{
	enum class AudioAPI
	{
		None,
		XAudio2
	};

	class VT_API AudioManager
	{
		public:
			static void Initialize();

			inline static AudioAPI GetAPI() noexcept { return api; }

			inline static Ref<IAudioContext> GetAudioContext() noexcept { return audioContext; }

		private:
			static AudioAPI api;
			static Ref<IAudioContext> audioContext;
	};
}