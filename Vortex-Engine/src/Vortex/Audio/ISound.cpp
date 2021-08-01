//
// Created by Vitriol1744 on 31.06.2021.
//
#include "ISound.hpp"

#include "Vortex/Audio/AudioManager.hpp"
#include "Vortex/Audio/XAudio2/XA2Sound.hpp"

namespace Vortex::Audio
{
	Ref<ISound> ISound::Create(Ref<ISoundBuffer> buffer)
	{
		Ref<ISound> result;

		switch (AudioManager::GetAPI())
		{
			case AudioAPI::XAudio2:
				result = CreateRef<XA2Sound>(buffer);
				break;

			default:
				result = nullptr;
				break;
		}

		return result;
	}

	Ref<ISound> ISound::Create(std::string_view filepath)
	{
	
		Ref<ISoundBuffer> soundBuffer = ISoundBuffer::Create(filepath);

        return Create(soundBuffer);
	}
}