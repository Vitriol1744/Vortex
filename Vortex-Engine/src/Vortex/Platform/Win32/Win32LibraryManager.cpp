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
        std::size_t filename_Size = strlen(filename);
		char* library_FileName = new char[filename_Size + 1 + 4];
		strcpy_s(library_FileName, filename_Size + 5, filename);
		strcat_s(library_FileName, 4, ".dll");

		HMODULE hModule = LoadLibraryExA(filename, nullptr, 0);

		return hModule;
	}
	void LibraryManager::CloseLibrary(void* library)
	{
	    FreeLibrary(static_cast<HMODULE>(library));
	}
	
	void* LibraryManager::GetProcAddress(void* library, const char* procName)
	{
		return reinterpret_cast<void*>(::GetProcAddress(static_cast<HMODULE>(library), procName));
	}
}
#endif