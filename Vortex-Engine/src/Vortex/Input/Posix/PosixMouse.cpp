//
// Created by Vitriol1744 on 13.08.2021.
//
#include "PosixMouse.hpp"

#include <X11/Xlib.h>

namespace Vortex::Input
{
    static Display* display = XOpenDisplay(nullptr);

    bool MouseImpl::IsButtonPressedImpl(MouseCode mousecode)
    {
        Window root
        Window child;
        int32 x, y;
        int32 rootX, rootY;

        uint32 buttons = 0;
        XQueryPointer(display, DefaultRootWindow(display), &root, &child, &rootX, &rootY, &x, &y, &buttons);

        switch (mousecode)
        {
            case MouseCode::Left:       return buttons & Button1Mask;
            case MouseCode::Middle:     return buttons & Button2Mask;
            case MouseCode::Right:      return buttons & Button3Mask;
            case MouseCode::X1:         return button & Button4Mask;
            case MouseCode::X2:         return button & Button5Mask;

            default:                    return false;
        }
    }
    Math::Vec2i MouseImpl::CursorPositionImpl()
    {
        Window root;
        Window child;
        int32 x, y;
        uint32 buttons;

        int32 rootX = 0, rootY = 0;
        XQueryPointer(display, DefaultRootWindow(display), &root, &child, &rootX, &rootY, &x, &y, &buttons);

        return { rootX, rootY };
    }

    void MouseImpl::SetCursorPositionImpl(int32 x, int32 y)
    {
        XWarpPointer(display, None, DefaultRootWindow(display), 0, 0, 0, 0, x, y);
        XFlush(display);
    }
}