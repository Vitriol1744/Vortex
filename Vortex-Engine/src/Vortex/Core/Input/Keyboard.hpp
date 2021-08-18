//
// Created by Vitriol1744 on 04.08.2021.
//
#pragma once

#include "Vortex/Core/Core.hpp"

#include "KeyCode.hpp"

namespace Vortex::Input
{
	class VT_API Keyboard
	{
		public:
			static bool IsKeyPressed(KeyCode keycode);
	};
}