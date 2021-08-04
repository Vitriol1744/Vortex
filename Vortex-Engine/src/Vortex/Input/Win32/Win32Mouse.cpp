//
// Created by Vitriol1744 on 04.08.2021.
//
#include "Win32Mouse.hpp"

#include <Windows.h>

namespace Vortex::Input
{
	static int32 Win32MouseCode(MouseCode mousecode)
	{
		switch (mousecode)
		{
			case MouseCode::Left:	return VK_LBUTTON;
			case MouseCode::Middle: return VK_MBUTTON;
			case MouseCode::Right:	return VK_RBUTTON;
			case MouseCode::X1:		return VK_XBUTTON1;
			case MouseCode::X2:		return VK_XBUTTON2;
		}

		return 0;
	}

	bool MouseImpl::IsButtonPressedImpl(MouseCode mousecode)
	{
		return (GetAsyncKeyState(Win32MouseCode(mousecode)) & 0x8000) != 0;
	}

	Math::Vec2i MouseImpl::CursorPositionImpl()
	{
		POINT mousePos;
		GetCursorPos(&mousePos);

		return { mousePos.x, mousePos.y };
	}

	void MouseImpl::SetCursorPositionImpl(int32 x, int32 y)
	{
		SetCursorPos(x, y);
	}
}