/*
 * Created by v1tr10l7 on 19.09.2024.
 * Copyright (c) 2024-2024, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#include <Prism/Containers/Array.hpp>

#include <Vortex/Window/Input/Mouse.hpp>
#include <Vortex/Window/Window.hpp>

namespace Vortex
{
    namespace Mouse
    {
        using Input::MouseCode;
        static Array<bool, ToUnderlying(MouseCode::eButtonCount)> s_Buttons{};
        static Math::Vec2d s_MousePosition{};

        void               Initialize()
        {
            Memory::Fill(s_Buttons.Raw(), false, s_Buttons.Size());
            WindowEvents::MouseButtonPressedEvent +=
                [](Window*, MouseCode button) -> bool
            {
                s_Buttons[ToUnderlying(button)] = true;
                return false;
            };

            WindowEvents::MouseButtonReleasedEvent +=
                [](Window*, MouseCode button) -> bool
            {
                s_Buttons[ToUnderlying(button)] = false;
                return false;
            };

            WindowEvents::MouseMovedEvent +=
                [](Window*, f64 xOffset, f64 yOffset) -> bool
            {
                s_MousePosition = Math::Vec2d(xOffset, yOffset);
                return false;
            };
        };

        bool IsButtonPressed(MouseCode mouseCode)
        {
            return s_Buttons[ToUnderlying(mouseCode)];
        }
        Math::Vec2d Position() { return s_MousePosition; }
    }; // namespace Mouse
}; // namespace Vortex
