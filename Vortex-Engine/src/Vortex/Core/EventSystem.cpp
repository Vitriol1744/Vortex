//
// Created by vitriol1744 on 08.07.2021.
//
#include "EventSystem.hpp"
#include "Vortex/Core/Event.hpp"

namespace Vortex
{
    void EventSystem::PushEvent(IEventBase* event)
    {
        eventBus.push(event);
    }

    void EventSystem::PollEvents()
    {
        while (!eventBus.empty())
        {
             eventBus.front()->Dispatch();
             eventBus.pop();
        }
    }
}