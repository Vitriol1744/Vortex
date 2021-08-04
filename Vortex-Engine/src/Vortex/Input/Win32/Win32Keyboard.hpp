//
// Created by Vitriol1744 on 04.08.2021.
//
#pragma once

#include "Vortex/Core/Core.hpp"

#include "Vortex/Input/Keyboard.hpp"

namespace Vortex::Input
{
	class VT_API KeyboardImpl final : public Keyboard
	{
		public:
			virtual bool IsKeyPressedImpl(KeyCode keycode) const override;
	};
}