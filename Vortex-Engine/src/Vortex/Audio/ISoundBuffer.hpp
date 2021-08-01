//
// Created by Vitriol1744 on 31.06.2021.
//
#pragma once

#include "Vortex/Core/Core.hpp"

#include <string_view>

namespace Vortex::Audio
{
	class VT_API ISoundBuffer
	{
		public:
			virtual ~ISoundBuffer() = default;

			static Ref<ISoundBuffer> Create(std::string_view filepath);
	};
}