//
// Created by Vitriol1744 on 31.06.2021.
//
#include "Vortex/Core/Core.hpp"

#ifdef VT_PLATFORM_WINDOWS
	#include <Windows.h>
#endif

namespace Vortex
{
	bool Platform::Initialize()
	{
		#ifdef VT_PLATFORM_WINDOWS
		HRESULT hr;
		hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);

		if (hr != S_OK)
		{
			if (hr == S_FALSE)
			{
				VT_CORE_LOG_WARN("Platform Already Initialized!");
			}
			else
			{
				VT_CORE_LOG_FATAL("Failed to Initialize Platform!");
				return false;
			}
		}
		#endif

		return true;
	}
	
	bool Shutdown()
	{
		CoUninitialize();
	
		return true;
	}
}