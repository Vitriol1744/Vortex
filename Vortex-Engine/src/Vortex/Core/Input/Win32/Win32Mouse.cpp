//
// Created by Vitriol1744 on 04.08.2021.
//
#include "Vortex/Platform/Platform.hpp"

#ifdef VT_PLATFORM_WINDOWS
#include "Vortex/Core/Input/Mouse.hpp"
#include <Windows.h>

namespace Vortex::Input
{
	static int32 Win32MouseCode(MouseCode mousecode)
	{
		switch (mousecode)
		{
			case MouseCode::eLeft:	return VK_LBUTTON;
			case MouseCode::eMiddle: return VK_MBUTTON;
			case MouseCode::eRight:	return VK_RBUTTON;
			case MouseCode::eX1:		return VK_XBUTTON1;
			case MouseCode::eX2:		return VK_XBUTTON2;
		}

		return 0;
	}

	bool Mouse::IsButtonPressed(MouseCode mousecode)
	{
		return (GetAsyncKeyState(Win32MouseCode(mousecode)) & 0x8000) != 0;
	}

	Math::Vec2i Mouse::CursorPosition()
	{
		POINT mousePos;
		GetCursorPos(&mousePos);

		return { mousePos.x, mousePos.y };
	}

	void Mouse::SetCursorPosition(int32 x, int32 y)
	{
		SetCursorPos(x, y);
	}
}
#endif