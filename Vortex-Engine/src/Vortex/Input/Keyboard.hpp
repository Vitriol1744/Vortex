//
// Created by Vitriol1744 on 04.08.2021.
//
#pragma once

#include "Vortex/Core/Core.hpp"

#include "Vortex/Input/KeyCode.hpp"

namespace Vortex::Input
{
	class VT_API Keyboard
	{
		public:
			inline static bool IsKeyPressed(KeyCode keycode) { return Instance()->IsKeyPressedImpl(keycode); }
	
		protected:
			virtual bool IsKeyPressedImpl(KeyCode keycode) const = 0;

		private:
			static Keyboard* Instance();
	};
}