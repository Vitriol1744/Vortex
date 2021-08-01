//
// Created by Vitriol1744 on 31.06.2021.
//
#pragma once

#include "Vortex/Core/Core.hpp"

namespace Vortex::Audio
{
	class VT_API IAudioContext
	{
		public:
			IAudioContext() = default;
			virtual ~IAudioContext() = default;

			static Ref<IAudioContext> Create();
	};
}