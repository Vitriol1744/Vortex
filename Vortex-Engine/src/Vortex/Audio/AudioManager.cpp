//
// Created by Vitriol1744 on 31.06.2021.
//
#include "AudioManager.hpp"

namespace Vortex::Audio
{
	AudioAPI AudioManager::api = AudioAPI::None;
	Ref<IAudioContext> AudioManager::audioContext = nullptr;

	void AudioManager::Initialize()
	{
		#ifdef VT_PLATFORM_WINDOWS
			api = AudioAPI::XAudio2;
		#else
			VT_CORE_LOG_WARN("Audio Is Currently Not Supported on this Platform!");
		#endif
		
		audioContext = IAudioContext::Create();
	}
}