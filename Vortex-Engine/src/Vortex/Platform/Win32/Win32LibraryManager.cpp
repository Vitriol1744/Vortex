//
// Created by Vitriol1744 on 24.08.2021.
//
#include "Vortex/Platform/LibraryManager.hpp"

#ifdef VT_PLATFORM_WINDOWS
#include <Windows.h>

#include <cstring>

namespace Vortex::Platform
{
	void* LibraryManager::LoadLibrary(const char* filename)
	{
		char* library_FileName = new char[strlen(filename) + 1 + 4];
		std::strcpy(library_FileName, filename);
		std::strcat(library_FileName, ".dll");

		HMODULE hModule = LoadLibraryExA(filename, nullptr, 0);

		return hModule;
	}
	void* LibraryManager::GetProcAddress(void* library, const char* procName)
	{
		return GetProcAddress(library, procName);
	}
}
#endif