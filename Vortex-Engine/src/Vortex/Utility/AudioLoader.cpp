//
// Created by Vitriol1744 on 30.06.2021.
//
#include "AudioLoader.hpp"

#include <fstream>
#include <cstring>

#ifdef VT_PLATFORM_XBOX //Big-Endian
	#define fourccRIFF 'RIFF'
	#define fourccDATA 'data'
	#define fourccFMT 'fmt '
	#define fourccWAVE 'WAVE'
	#define fourccXWMA 'XWMA'
	#define fourccDPDS 'dpds'
#else //Little-Endian
	#define fourccRIFF 'FFIR'
	#define fourccDATA 'atad'
	#define fourccFMT ' tmf'
	#define fourccWAVE 'EVAW'
	#define fourccXWMA 'AMWX'
	#define fourccDPDS 'sdpd'
#endif

namespace Vortex::Utility
{
	uint8* AudioLoader::LoadWAV(std::string_view filepath, uint32& size, WAVFormat& format)
	{
		std::ifstream ifs(filepath.data(), std::ios::binary);
		uint8* data = nullptr;
		if (!ifs.is_open())
		{
			VT_CORE_LOG_WARN("Failed to Open WAV file at path: {}", filepath);
			return nullptr;
		}

		uint32 chunkID;
		uint32 chunkSize;
		uint32 fileType;
		while (!ifs.eof())
		{
			ifs.read((char*)&chunkID, sizeof(uint32));
			ifs.read((char*)&chunkSize, sizeof(uint32));

			switch (chunkID)
			{
				case fourccRIFF:
					chunkSize = 4;
					ifs.read((char*)&fileType, sizeof(uint32));

					if (std::strncmp((char*)&fileType, "WAVE", 4) != 0)
					{
						VT_CORE_LOG_WARN("FileType is not WAV! filepath: {}", filepath);
						return nullptr;
					}
					break;
				case fourccFMT:
				{
					uint32 pos = ifs.tellg();
					ifs.read((char*)&format, sizeof(format));
					ifs.seekg(chunkSize + pos, std::ios_base::beg);
					break;
				}
				case fourccDATA:
					size = chunkSize;
					data = new uint8[size];
					ifs.read((char*)data, size);
					return data;
					break;

				default:
					ifs.seekg(chunkSize, std::ios_base::cur);
					break;
			}
		}

		return data;
	}
}