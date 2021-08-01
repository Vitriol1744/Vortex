//
// Created by Vitriol1744 on 31.06.2021.
//
#include "XA2Sound.hpp"

#include "Vortex/Audio/AudioManager.hpp"
#include "Vortex/Audio/XAudio2/XA2AudioContext.hpp"

namespace Vortex::Audio
{
	XA2Sound::XA2Sound(Ref<ISoundBuffer> buffer)
	{
		this->buffer = std::dynamic_pointer_cast<XA2SoundBuffer>(buffer);

		HRESULT hr;
		Ref<XA2AudioContext> context = std::dynamic_pointer_cast<XA2AudioContext>(AudioManager::GetAudioContext());

		WAVEFORMATEXTENSIBLE xformat = this->buffer->GetFormat();
		if (FAILED(hr = context->GetAudioEngine()->CreateSourceVoice(&sourceVoice, (WAVEFORMATEX*)&(xformat))))
		{
			VT_CORE_LOG_ERROR("failed to create source voice!");
		}

		XAUDIO2_BUFFER xbuffer = this->buffer->GetBuffer();
		if (FAILED(hr = sourceVoice->SubmitSourceBuffer(&(xbuffer)))) VT_CORE_LOG_ERROR("failed to submit!");
	}

	XA2Sound::~XA2Sound()
	{
		sourceVoice->Stop();
	}

	void XA2Sound::Play()
	{
		sourceVoice->Start(0);
	}
}