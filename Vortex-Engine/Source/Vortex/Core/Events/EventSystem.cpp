/*
 * Created by v1tr10l7 on 18.07.2024.
 * Copyright (c) 2024-2024, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#include <Vortex/Core/Events/Event.hpp>
#include <Vortex/Core/Events/EventSystem.hpp>

namespace Vortex::EventSystem
{
    static Queue<EventBase*> s_EventBus;
    void PushEvent(EventBase* event) { s_EventBus.Push(event); }

    void PollEvents()
    {
        while (!s_EventBus.Empty())
        {
            s_EventBus.Front()->Dispatch();
            s_EventBus.Pop();
        }
    }
} // namespace Vortex::EventSystem
