//
// Created by Vitriol1744 on 13.08.2021.
//
#include "Vortex/Core/Input/Keyboard.hpp"

#include <X11/Xlib.h>
#include <X11/keysym.h>

namespace Vortex::Input
{
    static KeySym XKeyCode(Input::KeyCode keycode)
    {
        switch (keycode)
        {
            case KeyCode::Num0:         return XK_0;
            case KeyCode::Num1:         return XK_1;
            case KeyCode::Num2:         return XK_2;
            case KeyCode::Num3:         return XK_3;
            case KeyCode::Num4:         return XK_4;
            case KeyCode::Num5:         return XK_5;
            case KeyCode::Num6:         return XK_6;
            case KeyCode::Num7:         return XK_7;
            case KeyCode::Num8:         return XK_8;
            case KeyCode::Num9:         return XK_9;
            case KeyCode::A:            return XK_a;
            case KeyCode::B:            return XK_b;
            case KeyCode::C:            return XK_c;
            case KeyCode::D:            return XK_d;
            case KeyCode::E:            return XK_e;
            case KeyCode::F:            return XK_f;
            case KeyCode::G:            return XK_g;
            case KeyCode::H:            return XK_h;
            case KeyCode::I:            return XK_i;
            case KeyCode::J:            return XK_j;
            case KeyCode::K:            return XK_k;
            case KeyCode::L:            return XK_l;
            case KeyCode::M:            return XK_m;
            case KeyCode::N:            return XK_n;
            case KeyCode::O:            return XK_o;
            case KeyCode::P:            return XK_p;
            case KeyCode::Q:            return XK_q;
            case KeyCode::R:            return XK_r;
            case KeyCode::S:            return XK_s;
            case KeyCode::T:            return XK_t;
            case KeyCode::U:            return XK_u;
            case KeyCode::V:            return XK_v;
            case KeyCode::W:            return XK_w;
            case KeyCode::X:            return XK_x;
            case KeyCode::Y:            return XK_y;
            case KeyCode::Z:            return XK_z;
            case KeyCode::Tilde:        return XK_grave;
            case KeyCode::F1:           return XK_F1;
            case KeyCode::F2:           return XK_F2;
            case KeyCode::F3:           return XK_F3;
            case KeyCode::F4:           return XK_F4;
            case KeyCode::F5:           return XK_F5;
            case KeyCode::F6:           return XK_F6;
            case KeyCode::F7:           return XK_F7;
            case KeyCode::F8:           return XK_F8;
            case KeyCode::F9:           return XK_F9;
            case KeyCode::F10:          return XK_F10;
            case KeyCode::F11:          return XK_F11;
            case KeyCode::F12:          return XK_F12;
            case KeyCode::F13:          return XK_F13;
            case KeyCode::F14:          return XK_F14;
            case KeyCode::F15:          return XK_F15;
            case KeyCode::Escape:       return XK_Escape;
            case KeyCode::BackSpace:    return XK_BackSpace;
            case KeyCode::Tab:          return XK_Tab;
            case KeyCode::CapsLock:     return XK_Caps_Lock;
            case KeyCode::Enter:        return XK_KP_Enter;
            case KeyCode::Return:       return XK_Return;
            case KeyCode::LShift:       return XK_Shift_L;
            case KeyCode::RShift:       return XK_Shift_R;
            case KeyCode::LCtrl:        return XK_Control_L;
            case KeyCode::RCtrl:        return XK_Control_R;
            case KeyCode::LAlt:         return XK_Alt_L;
            case KeyCode::RAlt:         return XK_Alt_R;
            case KeyCode::LSystem:      return XK_Super_L;
            case KeyCode::RSystem:      return XK_Super_R;
            case KeyCode::Space:        return XK_space;
            case KeyCode::Hyphen:       return XK_minus;
            case KeyCode::Equal:        return XK_equal;
            case KeyCode::Decimal:      return XK_KP_Decimal;
            case KeyCode::LBracket:     return XK_bracketleft;
            case KeyCode::RBracket:     return XK_bracketright;
            case KeyCode::Semicolon:    return XK_semicolon;
            case KeyCode::Apostrophe:   return XK_apostrophe;
            case KeyCode::Comma:        return XK_comma;
            case KeyCode::Period:       return XK_period;
            case KeyCode::Slash:        return XK_slash;
            case KeyCode::BackSlash:    return XK_backslash;
            case KeyCode::Up:           return XK_Up;
            case KeyCode::Down:         return XK_Down;
            case KeyCode::Left:         return XK_Left;
            case KeyCode::Right:        return XK_Right;
            case KeyCode::Numpad0:      return XK_KP_Insert;
            case KeyCode::Numpad1:      return XK_KP_End;
            case KeyCode::Numpad2:      return XK_KP_Down;
            case KeyCode::Numpad3:      return XK_KP_Page_Down;
            case KeyCode::Numpad4:      return XK_KP_Left;
            case KeyCode::Numpad5:      return XK_KP_Begin;
            case KeyCode::Numpad6:      return XK_KP_Right;
            case KeyCode::Numpad7:      return XK_KP_Home;
            case KeyCode::Numpad8:      return XK_KP_Up;
            case KeyCode::Numpad9:      return XK_KP_Page_Up;
            case KeyCode::Separator:    return XK_KP_Separator;
            case KeyCode::Add:          return XK_KP_Add;
            case KeyCode::Subtract:     return XK_KP_Subtract;
            case KeyCode::Multiply:     return XK_KP_Multiply;
            case KeyCode::Divide:       return XK_KP_Divide;
            case KeyCode::Insert:       return XK_Insert;
            case KeyCode::Delete:       return XK_Delete;
            case KeyCode::PageUp:       return XK_Page_Up;
            case KeyCode::PageDown:     return XK_Page_Down;
            case KeyCode::Home:         return XK_Home;
            case KeyCode::End:          return XK_End;
            
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
