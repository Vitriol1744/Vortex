/*
 * Created by v1tr10l7 on 05.09.2024.
 * Copyright (c) 2024-2024, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#include <Prism/Containers/Array.hpp>
#include <Vortex/Window/Input/Keyboard.hpp>
#include <Vortex/Window/Window.hpp>

namespace Vortex::Keyboard
{
    using namespace Vortex::Input;
    static Array<bool, ToUnderlying(KeyCode::eKeyCount)> s_Keys{};

    static bool OnKeyPressed(Window*, KeyCode key, u32)
    {
        s_Keys[ToUnderlying(key)] = true;

        return false;
    }
    static bool OnKeyReleased(Window*, KeyCode key)
    {
        s_Keys[ToUnderlying(key)] = false;

        return false;
    }

    void Initialize()
    {
        WindowEvents::KeyPressedEvent += OnKeyPressed;
        WindowEvents::KeyReleasedEvent += OnKeyReleased;
        Memory::Fill(s_Keys.Raw(), false, s_Keys.Size());
    }

    bool IsKeyPressed(KeyCode key) { return s_Keys[ToUnderlying(key)]; }
}; // namespace Vortex::Keyboard
