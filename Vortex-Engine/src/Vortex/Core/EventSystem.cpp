//
// Created by vitriol1744 on 08.07.2021.
//
#include "EventSystem.hpp"
#include "Core/IEvent.hpp"

namespace Vortex
{
    void EventSystem::PushEvent(IEventBase* event)
    {
        eventBus.push(event);
    }

    void EventSystem::Update()
    {
        while (!eventBus.empty())
        {
            //TODO: Smart pointers?
            eventBus.front()->Dispatch();
            //delete eventBus.front();
            eventBus.pop();
        }
    }
}