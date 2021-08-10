//
// Created by Vitriol1744 on 30.06.2021.
//
#pragma once
#include "Vortex/Core/Core.hpp"

#include <string_view>

namespace Vortex::Utility
{
	struct WAVFormat
	{
		uint16 formatType;
		uint16 channelsCount;
		uint32 sampleRate;
		uint32 bytesPerSecond;
		uint16 blockAlign;
		uint16 bitsPerSample;
		uint16 size;
		uint16 reserved;
		uint32 channels;
		unsigned long  data1;
		unsigned short data2;
		unsigned short data3;
		unsigned char  data4[8];
	};

	class VT_API AudioLoader
	{
		public:
			static uint8* LoadWAV(std::string_view filepath, uint32& size, WAVFormat& format);
	};
}