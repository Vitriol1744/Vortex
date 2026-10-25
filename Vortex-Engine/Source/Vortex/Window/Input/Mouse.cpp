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

        void VT_WINDOW_API Initialize(EventBus* eventBus)
        {
            Memory::Fill(s_Buttons.Raw(), false, s_Buttons.Size());

            eventBus->Subscribe<MouseButtonPressedEvent>(
                [](const MouseButtonPressedEvent& e)
                { s_Buttons[ToUnderlying(e.Button)] = true; });

            eventBus->Subscribe<MouseButtonReleasedEvent>(
                [](const MouseButtonReleasedEvent& e)
                { s_Buttons[ToUnderlying(e.Button)] = false; });

            eventBus->Subscribe<MouseMovedEvent>(
                [](const MouseMovedEvent& e)
                { s_MousePosition = Math::Vec2d(e.X, e.Y); });
        };

        bool VT_WINDOW_API IsButtonPressed(MouseCode mouseCode)
        {
            return s_Buttons[ToUnderlying(mouseCode)];
        }
        Math::Vec2d VT_WINDOW_API Position() { return s_MousePosition; }
    }; // namespace Mouse
}; // namespace Vortex
