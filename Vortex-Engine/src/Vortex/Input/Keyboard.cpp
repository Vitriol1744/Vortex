//
// Created by Vitriol1744 on 04.08.2021.
//
#include "Keyboard.hpp"

#ifdef VT_PLATFORM_WINDOWS
	#include "Vortex/Input/Win32/Win32Keyboard.hpp"
#elif defined(VT_PLATFORM_LINUX)
    #include "Vortex/Input/Posix/PosixKeyboard.hpp"
#endif

namespace Vortex::Input
{
	Keyboard* Keyboard::Instance()
	{
	    //TODO: Input Polling for Linux
		static Keyboard* instance = new KeyboardImpl;

		return instance;
	}
}