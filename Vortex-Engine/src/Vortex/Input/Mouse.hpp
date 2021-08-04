//
// Created by Vitriol1744 on 04.08.2021.
//
#pragma once

#include "Vortex/Core/Core.hpp"

#include "Vortex/Input/MouseCode.hpp"

namespace Vortex::Input
{
	class VT_API Mouse
	{
		public:
			inline static bool IsButtonPressed(MouseCode mousecode) { return Instance()->IsButtonPressedImpl(mousecode); }
			inline static Math::Vec2i CursorPosition() { return Instance()->CursorPositionImpl(); }

			inline static void SetCursorPosition(Math::Vec2i pos) { Instance()->SetCursorPositionImpl(pos.x, pos.y); }
			inline static void SetCursorPosition(int32 x, int32 y) { Instance()->SetCursorPositionImpl(x, y); }

		protected:
			virtual bool IsButtonPressedImpl(MouseCode mousecode) = 0;
			virtual Math::Vec2i CursorPositionImpl() = 0;

			virtual void SetCursorPositionImpl(int32 x, int32 y) = 0;
			
		private:
			static Mouse* Instance();
	};
}