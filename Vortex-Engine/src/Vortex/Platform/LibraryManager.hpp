//
// Created by Vitriol1744 on 24.08.2021.
//
#pragma once

#include "Vortex/Core/Core.hpp"
#undef LoadLibrary

namespace Vortex::Platform
{
	class VT_API LibraryManager
	{
		public:
			static void* LoadLibrary(const char* filename);
			static void* GetProcAddress(void* library, const char* procName);
	};
}