/*
 * Created by v1tr10l7 on 19.09.2024.
 * Copyright (c) 2024-2024, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
import Prism.Containers.Array;

#include <Vortex/Window/Input/Mouse.hpp>
#include <Vortex/Window/Window.hpp>

namespace Vortex
{
    namespace Mouse
    {
        using Input::MouseCode;
        static Array<bool, ToUnderlying(MouseCode::eButtonCount)> s_Buttons{};
        static Math::Vec2d s_MousePosition{};
        static Math::Vec2d s_Delta{};
        /// Raw motion arrives: position differences would count it twice.
        static bool        s_HaveRawMotion = false;

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
                {
                    Math::Vec2d position(e.X, e.Y);
                    if (!s_HaveRawMotion) s_Delta += position - s_MousePosition;
                    s_MousePosition = position;
                });
        };

        bool VT_WINDOW_API IsButtonPressed(MouseCode mouseCode)
        {
            return s_Buttons[ToUnderlying(mouseCode)];
        }
        Math::Vec2d VT_WINDOW_API Position() { return s_MousePosition; }

        Math::Vec2d VT_WINDOW_API ConsumeDelta()
        {
            Math::Vec2d delta = s_Delta;
            s_Delta           = Math::Vec2d(0.0);
            return delta;
        }
        void VT_WINDOW_API AddRawDelta(f64 dx, f64 dy)
        {
            s_HaveRawMotion = true;
            s_Delta += Math::Vec2d(dx, dy);
        }
    }; // namespace Mouse
}; // namespace Vortex
