//
// Created by Vitriol1744 on 31.06.2021.
//
#include "XA2AudioContext.hpp"

namespace Vortex::Audio
{
	XA2AudioContext::XA2AudioContext()
	{
		HRESULT hr;
		if (FAILED(hr = XAudio2Create(&xAudio2, 0, XAUDIO2_DEFAULT_PROCESSOR))) VT_CORE_LOG_ERROR("Failed to Create XAudio2 Engine!");
		if (FAILED(hr = xAudio2->CreateMasteringVoice(&masterVoice))) VT_CORE_LOG_ERROR("Failed to Create XAudio2 Mastering Voice!");
	}
}