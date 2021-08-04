//
// Created by Vitriol1744 on 03.08.2021.
//
#include "Gamepad.hpp"

#ifdef VT_PLATFORM_WINDOWS
	#include "Vortex/Input/Win32/Win32Gamepad.hpp"
#endif

namespace Vortex::Input
{
	Event<GamepadButton> Gamepad::buttonPressedEvent{};
	Event<GamepadButton> Gamepad::buttonReleasedEvent{};

	Gamepad* Gamepad::Instance()
	{
		static Gamepad* instance = new GamepadImpl;

		return instance;
	}
}