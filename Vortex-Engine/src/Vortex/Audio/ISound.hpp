//
// Created by Vitriol1744 on 31.06.2021.
//
#pragma once

#include "Vortex/Core/Core.hpp"

#include "Vortex/Audio/ISoundBuffer.hpp"

namespace Vortex::Audio
{
	class VT_API ISound
	{
		public:
			virtual ~ISound() = default;

			static Ref<ISound> Create(std::string_view filepath);
			static Ref<ISound> Create(Ref<ISoundBuffer> buffer);

			virtual void Play() = 0;
	};
}