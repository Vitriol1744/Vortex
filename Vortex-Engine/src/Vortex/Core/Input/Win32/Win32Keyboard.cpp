//
// Created by Vitriol1744 on 04.08.2021.
//
#include "Vortex/Platform/Platform.hpp"

#ifdef VT_PLATFORM_WINDOWS
#include "Vortex/Core/Input/Keyboard.hpp"
#include <Windows.h>

namespace Vortex::Input
{
	static uint32 Win32KeyCode(KeyCode keycode)
	{
        switch (keycode)
        {
            case KeyCode::Num0:             return '0';
            case KeyCode::Num1:             return '1';
            case KeyCode::Num2:             return '2';
            case KeyCode::Num3:             return '3';
            case KeyCode::Num4:             return '4';
            case KeyCode::Num5:             return '5';
            case KeyCode::Num6:             return '6';
            case KeyCode::Num7:             return '7';
            case KeyCode::Num8:             return '8';
            case KeyCode::Num9:             return '9';
            case KeyCode::A:                return 'A';
            case KeyCode::B:                return 'B';
            case KeyCode::C:                return 'C';
            case KeyCode::D:                return 'D';
            case KeyCode::E:                return 'E';
            case KeyCode::F:                return 'F';
            case KeyCode::G:                return 'G';
            case KeyCode::H:                return 'H';
            case KeyCode::I:                return 'I';
            case KeyCode::J:                return 'J';
            case KeyCode::K:                return 'K';
            case KeyCode::L:                return 'L';
            case KeyCode::M:                return 'M';
            case KeyCode::N:                return 'N';
            case KeyCode::O:                return 'O';
            case KeyCode::P:                return 'P';
            case KeyCode::Q:                return 'Q';
            case KeyCode::R:                return 'R';
            case KeyCode::S:                return 'S';
            case KeyCode::T:                return 'T';
            case KeyCode::U:                return 'U';
            case KeyCode::V:                return 'V';
            case KeyCode::W:                return 'W';
            case KeyCode::X:                return 'X';
            case KeyCode::Y:                return 'Y';
            case KeyCode::Z:                return 'Z';
            case KeyCode::Tilde:            return VK_OEM_3;
            case KeyCode::F1:               return VK_F1;
            case KeyCode::F2:               return VK_F2;
            case KeyCode::F3:               return VK_F3;
            case KeyCode::F4:               return VK_F4;
            case KeyCode::F5:               return VK_F5;
            case KeyCode::F6:               return VK_F6;
            case KeyCode::F7:               return VK_F7;
            case KeyCode::F8:               return VK_F8;
            case KeyCode::F9:               return VK_F9;
            case KeyCode::F10:              return VK_F10;
            case KeyCode::F11:              return VK_F11;
            case KeyCode::F12:              return VK_F12;
            case KeyCode::F13:              return VK_F13;
            case KeyCode::F14:              return VK_F14;
            case KeyCode::F15:              return VK_F15;
            case KeyCode::Escape:           return VK_ESCAPE;
            case KeyCode::BackSpace:        return VK_BACK;
            case KeyCode::Tab:              return VK_TAB;
            case KeyCode::CapsLock:         return VK_CAPITAL;
            case KeyCode::Return: 
            case KeyCode::Enter:            return VK_RETURN;
            case KeyCode::LShift:           return VK_LSHIFT;
            case KeyCode::RShift:           return VK_RSHIFT;
            case KeyCode::LCtrl:            return VK_LCONTROL;
            case KeyCode::RCtrl:            return VK_RCONTROL;
            case KeyCode::LAlt:             return VK_LMENU;
            case KeyCode::RAlt:             return VK_RMENU;
            case KeyCode::LSystem:          return VK_LWIN;
            case KeyCode::RSystem:          return VK_RWIN;
            case KeyCode::Space:            return VK_SPACE;
            case KeyCode::Hyphen:           return VK_OEM_MINUS;
            case KeyCode::Equal:            return VK_OEM_PLUS;
            case KeyCode::Decimal:          return VK_DECIMAL;
            case KeyCode::LBracket:         return VK_OEM_4;
            case KeyCode::RBracket:         return VK_OEM_6;
            case KeyCode::Semicolon:        return VK_OEM_1;
            case KeyCode::Apostrophe:       return VK_OEM_7;
            case KeyCode::Comma:            return VK_OEM_COMMA;
            case KeyCode::Period:           return VK_OEM_PERIOD;
            case KeyCode::Slash:            return VK_OEM_2;
            case KeyCode::BackSlash:        return VK_OEM_5;
            case KeyCode::Up:               return VK_UP;
            case KeyCode::Down:             return VK_DOWN;
            case KeyCode::Left:             return VK_LEFT;
            case KeyCode::Right:            return VK_RIGHT;
            case KeyCode::Numpad0:          return VK_NUMPAD0;
            case KeyCode::Numpad1:          return VK_NUMPAD1;
            case KeyCode::Numpad2:          return VK_NUMPAD2;
            case KeyCode::Numpad3:          return VK_NUMPAD3;
            case KeyCode::Numpad4:          return VK_NUMPAD4;
            case KeyCode::Numpad5:          return VK_NUMPAD5;
            case KeyCode::Numpad6:          return VK_NUMPAD6;
            case KeyCode::Numpad7:          return VK_NUMPAD7;
            case KeyCode::Numpad8:          return VK_NUMPAD8;
            case KeyCode::Numpad9:          return VK_NUMPAD9;
            case KeyCode::Separator:        return VK_SEPARATOR;
            case KeyCode::Add:              return VK_ADD;
            case KeyCode::Subtract:         return VK_SUBTRACT;
            case KeyCode::Multiply:         return VK_MULTIPLY;
            case KeyCode::Divide:           return VK_DIVIDE;
            case KeyCode::Insert:           return VK_INSERT;
            case KeyCode::Delete:           return VK_DELETE;
            case KeyCode::PageUp:           return VK_PRIOR;
            case KeyCode::PageDown:         return VK_NEXT;
            case KeyCode::Home:             return VK_HOME;
            case KeyCode::End:              return VK_END;
        }
        
        return 0;
	}

	bool Keyboard::IsKeyPressed(KeyCode keycode)
	{
		return (GetAsyncKeyState(Win32KeyCode(keycode)) & 0x8000) != 0;
	}
}
#endif