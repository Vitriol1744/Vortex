//
// Created by Vitriol1744 on 13.08.2021.
//
#include "Vortex/Core/Input/Keyboard.hpp"
#ifdef VT_PLATFORM_LINUX
#include <X11/Xlib.h>
#include <X11/keysym.h>

namespace Vortex::Input
{
    static KeySym XKeyCode(Input::KeyCode keycode)
    {
        switch (keycode)
        {
            case KeyCode::eNum0:         return XK_0;
            case KeyCode::eNum1:         return XK_1;
            case KeyCode::eNum2:         return XK_2;
            case KeyCode::eNum3:         return XK_3;
            case KeyCode::eNum4:         return XK_4;
            case KeyCode::eNum5:         return XK_5;
            case KeyCode::eNum6:         return XK_6;
            case KeyCode::eNum7:         return XK_7;
            case KeyCode::eNum8:         return XK_8;
            case KeyCode::eNum9:         return XK_9;
            case KeyCode::eA:            return XK_a;
            case KeyCode::eB:            return XK_b;
            case KeyCode::eC:            return XK_c;
            case KeyCode::eD:            return XK_d;
            case KeyCode::eE:            return XK_e;
            case KeyCode::eF:            return XK_f;
            case KeyCode::eG:            return XK_g;
            case KeyCode::eH:            return XK_h;
            case KeyCode::eI:            return XK_i;
            case KeyCode::eJ:            return XK_j;
            case KeyCode::eK:            return XK_k;
            case KeyCode::eL:            return XK_l;
            case KeyCode::eM:            return XK_m;
            case KeyCode::eN:            return XK_n;
            case KeyCode::eO:            return XK_o;
            case KeyCode::eP:            return XK_p;
            case KeyCode::eQ:            return XK_q;
            case KeyCode::eR:            return XK_r;
            case KeyCode::eS:            return XK_s;
            case KeyCode::eT:            return XK_t;
            case KeyCode::eU:            return XK_u;
            case KeyCode::eV:            return XK_v;
            case KeyCode::eW:            return XK_w;
            case KeyCode::eX:            return XK_x;
            case KeyCode::eY:            return XK_y;
            case KeyCode::eZ:            return XK_z;
            case KeyCode::eTilde:        return XK_grave;
            case KeyCode::eF1:           return XK_F1;
            case KeyCode::eF2:           return XK_F2;
            case KeyCode::eF3:           return XK_F3;
            case KeyCode::eF4:           return XK_F4;
            case KeyCode::eF5:           return XK_F5;
            case KeyCode::eF6:           return XK_F6;
            case KeyCode::eF7:           return XK_F7;
            case KeyCode::eF8:           return XK_F8;
            case KeyCode::eF9:           return XK_F9;
            case KeyCode::eF10:          return XK_F10;
            case KeyCode::eF11:          return XK_F11;
            case KeyCode::eF12:          return XK_F12;
            case KeyCode::eF13:          return XK_F13;
            case KeyCode::eF14:          return XK_F14;
            case KeyCode::eF15:          return XK_F15;
            case KeyCode::eEscape:       return XK_Escape;
            case KeyCode::eBackSpace:    return XK_BackSpace;
            case KeyCode::eTab:          return XK_Tab;
            case KeyCode::eCapsLock:     return XK_Caps_Lock;
            case KeyCode::eEnter:        return XK_KP_Enter;
            case KeyCode::eReturn:       return XK_Return;
            case KeyCode::eLShift:       return XK_Shift_L;
            case KeyCode::eRShift:       return XK_Shift_R;
            case KeyCode::eLCtrl:        return XK_Control_L;
            case KeyCode::eRCtrl:        return XK_Control_R;
            case KeyCode::eLAlt:         return XK_Alt_L;
            case KeyCode::eRAlt:         return XK_Alt_R;
            case KeyCode::eLSystem:      return XK_Super_L;
            case KeyCode::eRSystem:      return XK_Super_R;
            case KeyCode::eSpace:        return XK_space;
            case KeyCode::eHyphen:       return XK_minus;
            case KeyCode::eEqual:        return XK_equal;
            case KeyCode::eDecimal:      return XK_KP_Decimal;
            case KeyCode::eLBracket:     return XK_bracketleft;
            case KeyCode::eRBracket:     return XK_bracketright;
            case KeyCode::eSemicolon:    return XK_semicolon;
            case KeyCode::eApostrophe:   return XK_apostrophe;
            case KeyCode::eComma:        return XK_comma;
            case KeyCode::ePeriod:       return XK_period;
            case KeyCode::eSlash:        return XK_slash;
            case KeyCode::eBackSlash:    return XK_backslash;
            case KeyCode::eUp:           return XK_Up;
            case KeyCode::eDown:         return XK_Down;
            case KeyCode::eLeft:         return XK_Left;
            case KeyCode::eRight:        return XK_Right;
            case KeyCode::eNumpad0:      return XK_KP_Insert;
            case KeyCode::eNumpad1:      return XK_KP_End;
            case KeyCode::eNumpad2:      return XK_KP_Down;
            case KeyCode::eNumpad3:      return XK_KP_Page_Down;
            case KeyCode::eNumpad4:      return XK_KP_Left;
            case KeyCode::eNumpad5:      return XK_KP_Begin;
            case KeyCode::eNumpad6:      return XK_KP_Right;
            case KeyCode::eNumpad7:      return XK_KP_Home;
            case KeyCode::eNumpad8:      return XK_KP_Up;
            case KeyCode::eNumpad9:      return XK_KP_Page_Up;
            case KeyCode::eSeparator:    return XK_KP_Separator;
            case KeyCode::eAdd:          return XK_KP_Add;
            case KeyCode::eSubtract:     return XK_KP_Subtract;
            case KeyCode::eMultiply:     return XK_KP_Multiply;
            case KeyCode::eDivide:       return XK_KP_Divide;
            case KeyCode::eInsert:       return XK_Insert;
            case KeyCode::eDelete:       return XK_Delete;
            case KeyCode::ePageUp:       return XK_Page_Up;
            case KeyCode::ePageDown:     return XK_Page_Down;
            case KeyCode::eHome:         return XK_Home;
            case KeyCode::eEnd:          return XK_End;
            
            default:                    return 0;
        }
    }

    bool Keyboard::IsKeyPressed(Input::KeyCode keycode)
    {
        static Display* display = XOpenDisplay(nullptr);

        ::KeySym  keysym    = XKeyCode(keycode);
        ::KeyCode xkeycode  = XKeysymToKeycode(display, keysym);
        if (xkeycode)
        {
            char keys[32];
            XQueryKeymap(display, keys);

            return (keys[xkeycode / 8] & (1 << (xkeycode % 8))) != 0;
        }
        return false;
    }
}
#endif