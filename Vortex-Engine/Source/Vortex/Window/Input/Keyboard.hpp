/*
 * Created by v1tr10l7 on 04.09.2024.
 * Copyright (c) 2024-2024, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include <Vortex/Core/Events/EventBus.hpp>
#include <Vortex/Window/Export.hpp>
#include <Vortex/Window/Input/KeyCode.hpp>

namespace Vortex::Keyboard
{
    void VT_WINDOW_API Initialize(EventBus* eventBus);
    bool VT_WINDOW_API IsKeyPressed(Input::KeyCode key);
}; // namespace Vortex::Keyboard
