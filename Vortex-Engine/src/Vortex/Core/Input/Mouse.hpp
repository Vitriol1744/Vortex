//
// Created by Vitriol1744 on 04.08.2021.
//
#pragma once

#include "Vortex/Core/Core.hpp"

#include "MouseCode.hpp"

namespace Vortex::Input
{
	class VT_API Mouse
	{
		public:
			static bool IsButtonPressed(MouseCode mousecode);
			static Math::Vec2i CursorPosition();

			inline static void SetCursorPosition(Math::Vec2i pos) { SetCursorPosition(pos.x, pos.y); }
			static void SetCursorPosition(int32 x, int32 y);
	};
}