//
// Created by Vitriol1744 on 31.06.2021.
//
#include "IAudioContext.hpp"

#include "Vortex/Audio/AudioManager.hpp"
#include "Vortex/Audio/XAudio2/XA2AudioContext.hpp"

namespace Vortex::Audio
{
	Ref<IAudioContext> IAudioContext::Create()
	{
		Ref<IAudioContext> result;

		switch (AudioManager::GetAPI())
		{
			case AudioAPI::XAudio2:
				#ifdef VT_PLATFORM_WINDOWS
					result = CreateRef<XA2AudioContext>();
				#else
					VT_CORE_LOG_WARN("XAudio2 is supported only on Windows!");
				#endif
				break;

			default:
				result = nullptr;
				break;
		}

		return result;
	}
}