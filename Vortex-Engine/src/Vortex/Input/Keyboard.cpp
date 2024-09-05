/*
 * Created by v1tr10l7 on 05.09.2024.
 * Copyright (c) 2024-2024, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#include "vtpch.hpp"

#include "Vortex/Input/Keyboard.hpp"
#include "Vortex/Renderer/Window/Window.hpp"

namespace Vortex::Keyboard
{
    using namespace Vortex::Input;
    static std::array<bool, std::to_underlying(KeyCode::eKeyCount)> s_Keys{};

    static bool OnKeyPressed(Window*, KeyCode key, u32)
    {
        s_Keys[std::to_underlying(key)] = true;

        return false;
    }
    static bool OnKeyReleased(Window*, KeyCode key)
    {
        s_Keys[std::to_underlying(key)] = false;

        return false;
    }

    void Initialize()
    {
        WindowEvents::KeyPressedEvent += OnKeyPressed;
        WindowEvents::KeyReleasedEvent += OnKeyReleased;
        std::memset(s_Keys.data(), false, s_Keys.size());
    }

    bool IsKeyPressed(KeyCode key) { return s_Keys[std::to_underlying(key)]; }
}; // namespace Vortex::Keyboard
