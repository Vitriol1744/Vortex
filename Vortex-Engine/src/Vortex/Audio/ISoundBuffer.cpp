//
// Created by Vitriol1744 on 31.06.2021.
//
#include "ISoundBuffer.hpp"

#include "Vortex/Audio/AudioManager.hpp"

#ifdef VT_PLATFORM_WINDOWS
	#include "Vortex/Audio/XAudio2/XA2SoundBuffer.hpp"
#endif

namespace Vortex::Audio
{
	Ref<ISoundBuffer> ISoundBuffer::Create(std::string_view filepath)
	{
		Ref<ISoundBuffer> result;

		switch (AudioManager::GetAPI())
		{
			case AudioAPI::XAudio2:
				#ifdef VT_PLATFORM_WINDOWS
					result = CreateRef<XA2SoundBuffer>(filepath);
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