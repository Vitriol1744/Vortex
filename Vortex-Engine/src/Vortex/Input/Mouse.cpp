//
// Created by Vitriol1744 on 04.08.2021.
//
#include "Mouse.hpp"

#include "Vortex/Input/Win32/Win32Mouse.hpp"

namespace Vortex::Input
{
	Mouse* Mouse::Instance()
	{
		static Mouse* instance = new MouseImpl;

		return instance;
	}
}