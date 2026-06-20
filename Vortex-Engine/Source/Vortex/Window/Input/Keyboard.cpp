/*
 * Created by v1tr10l7 on 05.09.2024.
 * Copyright (c) 2024-2024, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#include <Prism/Containers/Array.hpp>
#include <Vortex/Engine/Application.hpp>
#include <Vortex/Window/Input/Keyboard.hpp>
#include <Vortex/Window/Window.hpp>

namespace Vortex::Keyboard
{
    using namespace Vortex::Input;
    static Array<bool, ToUnderlying(KeyCode::eKeyCount)> s_Keys{};

    static void OnKeyPressed(const KeyPressedEvent& e)
    {
        auto key                  = e.KeyCode;
        s_Keys[ToUnderlying(key)] = true;

        return;
    }
    static void OnKeyReleased(const KeyReleasedEvent& e)
    {
        auto key                  = e.KeyCode;
        s_Keys[ToUnderlying(key)] = false;

        return;
    }

    void Initialize()
    {
        Application::Get()->SubscribeEvent<KeyPressedEvent>(
            [](const KeyPressedEvent& e) { return OnKeyPressed(e); });
        Application::Get()->SubscribeEvent<KeyReleasedEvent>(
            [](const KeyReleasedEvent& e) { return OnKeyReleased(e); });
        Memory::Fill(s_Keys.Raw(), false, s_Keys.Size());
    }

    bool IsKeyPressed(KeyCode key) { return s_Keys[ToUnderlying(key)]; }
}; // namespace Vortex::Keyboard
