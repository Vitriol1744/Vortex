/*
 * Created by v1tr10l7 on 18.07.2024.
 * Copyright (c) 2024-2024, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#include "vtpch.hpp"

#include "Vortex/Core/Events/Event.hpp"
#include "Vortex/Core/Events/EventSystem.hpp"

namespace Vortex::EventSystem
{
    static std::queue<EventBase*> s_EventBus;
    void PushEvent(EventBase* event) { s_EventBus.push(event); }

    void PollEvents()
    {
        while (!s_EventBus.empty())
        {
            s_EventBus.front()->Dispatch();
            s_EventBus.pop();
        }
    }
} // namespace Vortex::EventSystem
