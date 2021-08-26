//
// Created by Vitriol1744 on 26.08.2021.
//
#pragma once

#include "Vortex/Core/Core.hpp"

namespace Vortex::Platform
{
	class VT_API MachineInfo
	{
		public:
			static uint64 GetTotalMemory();
			static uint64 GetAvailableMemory();
			static uint64 GetTotalMemoryUsage();
	};
}