/*
 * Created by v1tr10l7 on 18.07.2024.
 * Copyright (c) 2024-2024, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include "Vortex/Core/Export.hpp"

namespace Vortex
{
    class EventBase;

    namespace EventSystem
    {
        VT_API void PushEvent(EventBase* event);

        VT_API void PollEvents();
    }; // namespace EventSystem
} // namespace Vortex
