//
// Created by Vitriol1744 on 26.08.2021.
//
#include "Vortex/Platform/MachineInfo.hpp"

#ifdef VT_PLATFORM_WINDOWS
#include <Windows.h>

namespace Vortex::Platform
{
	uint64 MachineInfo::GetTotalMemory()
	{
		MEMORYSTATUSEX memoryStatus = {};
		GlobalMemoryStatusEx(&memoryStatus);

		return memoryStatus.ullTotalPhys;
	}
	uint64 MachineInfo::GetAvailableMemory()
	{
		MEMORYSTATUSEX memoryStatus = {};
		GlobalMemoryStatusEx(&memoryStatus);

		return memoryStatus.ullAvailPhys;
	}
	uint64 MachineInfo::GetTotalMemoryUsage()
	{
		return GetTotalMemory() - GetAvailableMemory();
	}
}
#endif