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
            case KeyCode::eNum0:             return '0';
            case KeyCode::eNum1:             return '1';
            case KeyCode::eNum2:             return '2';
            case KeyCode::eNum3:             return '3';
            case KeyCode::eNum4:             return '4';
            case KeyCode::eNum5:             return '5';
            case KeyCode::eNum6:             return '6';
            case KeyCode::eNum7:             return '7';
            case KeyCode::eNum8:             return '8';
            case KeyCode::eNum9:             return '9';
            case KeyCode::eA:                return 'A';
            case KeyCode::eB:                return 'B';
            case KeyCode::eC:                return 'C';
            case KeyCode::eD:                return 'D';
            case KeyCode::eE:                return 'E';
            case KeyCode::eF:                return 'F';
            case KeyCode::eG:                return 'G';
            case KeyCode::eH:                return 'H';
            case KeyCode::eI:                return 'I';
            case KeyCode::eJ:                return 'J';
            case KeyCode::eK:                return 'K';
            case KeyCode::eL:                return 'L';
            case KeyCode::eM:                return 'M';
            case KeyCode::eN:                return 'N';
            case KeyCode::eO:                return 'O';
            case KeyCode::eP:                return 'P';
            case KeyCode::eQ:                return 'Q';
            case KeyCode::eR:                return 'R';
            case KeyCode::eS:                return 'S';
            case KeyCode::eT:                return 'T';
            case KeyCode::eU:                return 'U';
            case KeyCode::eV:                return 'V';
            case KeyCode::eW:                return 'W';
            case KeyCode::eX:                return 'X';
            case KeyCode::eY:                return 'Y';
            case KeyCode::eZ:                return 'Z';
            case KeyCode::eTilde:            return VK_OEM_3;
            case KeyCode::eF1:               return VK_F1;
            case KeyCode::eF2:               return VK_F2;
            case KeyCode::eF3:               return VK_F3;
            case KeyCode::eF4:               return VK_F4;
            case KeyCode::eF5:               return VK_F5;
            case KeyCode::eF6:               return VK_F6;
            case KeyCode::eF7:               return VK_F7;
            case KeyCode::eF8:               return VK_F8;
            case KeyCode::eF9:               return VK_F9;
            case KeyCode::eF10:              return VK_F10;
            case KeyCode::eF11:              return VK_F11;
            case KeyCode::eF12:              return VK_F12;
            case KeyCode::eF13:              return VK_F13;
            case KeyCode::eF14:              return VK_F14;
            case KeyCode::eF15:              return VK_F15;
            case KeyCode::eEscape:           return VK_ESCAPE;
            case KeyCode::eBackSpace:        return VK_BACK;
            case KeyCode::eTab:              return VK_TAB;
            case KeyCode::eCapsLock:         return VK_CAPITAL;
            case KeyCode::eReturn:
            case KeyCode::eEnter:            return VK_RETURN;
            case KeyCode::eLShift:           return VK_LSHIFT;
            case KeyCode::eRShift:           return VK_RSHIFT;
            case KeyCode::eLCtrl:            return VK_LCONTROL;
            case KeyCode::eRCtrl:            return VK_RCONTROL;
            case KeyCode::eLAlt:             return VK_LMENU;
            case KeyCode::eRAlt:             return VK_RMENU;
            case KeyCode::eLSystem:          return VK_LWIN;
            case KeyCode::eRSystem:          return VK_RWIN;
            case KeyCode::eSpace:            return VK_SPACE;
            case KeyCode::eHyphen:           return VK_OEM_MINUS;
            case KeyCode::eEqual:            return VK_OEM_PLUS;
            case KeyCode::eDecimal:          return VK_DECIMAL;
            case KeyCode::eLBracket:         return VK_OEM_4;
            case KeyCode::eRBracket:         return VK_OEM_6;
            case KeyCode::eSemicolon:        return VK_OEM_1;
            case KeyCode::eApostrophe:       return VK_OEM_7;
            case KeyCode::eComma:            return VK_OEM_COMMA;
            case KeyCode::ePeriod:           return VK_OEM_PERIOD;
            case KeyCode::eSlash:            return VK_OEM_2;
            case KeyCode::eBackSlash:        return VK_OEM_5;
            case KeyCode::eUp:               return VK_UP;
            case KeyCode::eDown:             return VK_DOWN;
            case KeyCode::eLeft:             return VK_LEFT;
            case KeyCode::eRight:            return VK_RIGHT;
            case KeyCode::eNumpad0:          return VK_NUMPAD0;
            case KeyCode::eNumpad1:          return VK_NUMPAD1;
            case KeyCode::eNumpad2:          return VK_NUMPAD2;
            case KeyCode::eNumpad3:          return VK_NUMPAD3;
            case KeyCode::eNumpad4:          return VK_NUMPAD4;
            case KeyCode::eNumpad5:          return VK_NUMPAD5;
            case KeyCode::eNumpad6:          return VK_NUMPAD6;
            case KeyCode::eNumpad7:          return VK_NUMPAD7;
            case KeyCode::eNumpad8:          return VK_NUMPAD8;
            case KeyCode::eNumpad9:          return VK_NUMPAD9;
            case KeyCode::eSeparator:        return VK_SEPARATOR;
            case KeyCode::eAdd:              return VK_ADD;
            case KeyCode::eSubtract:         return VK_SUBTRACT;
            case KeyCode::eMultiply:         return VK_MULTIPLY;
            case KeyCode::eDivide:           return VK_DIVIDE;
            case KeyCode::eInsert:           return VK_INSERT;
            case KeyCode::eDelete:           return VK_DELETE;
            case KeyCode::ePageUp:           return VK_PRIOR;
            case KeyCode::ePageDown:         return VK_NEXT;
            case KeyCode::eHome:             return VK_HOME;
            case KeyCode::eEnd:              return VK_END;
        }
        
        return 0;
	}

	bool Keyboard::IsKeyPressed(KeyCode keycode)
	{
		return (GetAsyncKeyState(Win32KeyCode(keycode)) & 0x8000) != 0;
	}
}
#endif