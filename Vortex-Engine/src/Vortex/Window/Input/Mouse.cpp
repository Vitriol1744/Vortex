/*
 * Created by v1tr10l7 on 19.09.2024.
 * Copyright (c) 2024-2024, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#include "vtpch.hpp"

#include "Vortex/Window/Input/Mouse.hpp"
#include "Vortex/Window/Window.hpp"

namespace Vortex
{
    namespace Mouse
    {
        using Input::MouseCode;

        static std::array<bool, std::to_underlying(MouseCode::eButtonCount)>
                     s_Buttons{};
        static Vec2d s_MousePosition{};

        void         Initialize()
        {
            std::memset(s_Buttons.data(), false, s_Buttons.size());
            WindowEvents::MouseButtonPressedEvent +=
                [](Window*, MouseCode button) -> bool
            {
                s_Buttons[std::to_underlying(button)] = true;
                return false;
            };

            WindowEvents::MouseButtonReleasedEvent +=
                [](Window*, MouseCode button) -> bool
            {
                s_Buttons[std::to_underlying(button)] = false;
                return false;
            };

            WindowEvents::MouseMovedEvent +=
                [](Window*, f64 xOffset, f64 yOffset) -> bool
            {
                s_MousePosition = Vec2d(xOffset, yOffset);
                return false;
            };
        };

        bool IsButtonPressed(MouseCode mouseCode)
        {
            return s_Buttons[std::to_underlying(mouseCode)];
        }
        Vec2d GetPosition() { return s_MousePosition; }
    }; // namespace Mouse
};     // namespace Vortex
