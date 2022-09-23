//
// Created by Vitriol1744 on 08.09.2022.
//
#include "Vortex/vtpch.hpp"
#include "EventSystem.hpp"

#include "Vortex/Core/LoggingManager.hpp"

namespace Vortex
{
    void EventSystem::SubscribeEvent(EventID event_id, IListener* listener)
    {
        if (!listener)
        {
            VTCoreLogWarn("Listener for event_id: '{}' is 0", event_id);
            return;
        }

        listeners.insert(std::make_pair(event_id, listener));
    }
    void EventSystem::UnsubscribeEvent(EventID event_id, IListener* listener)
    {
        std::pair<std::multimap<EventID, IListener*>::iterator, std::multimap<EventID, IListener*>::iterator> range;
        range = listeners.equal_range(event_id);

        for (std::multimap<EventID, IListener*>::iterator iter = range.first; iter != range.second; iter++)
        {
            if ((*iter).second == listener)
            {
                iter = listeners.erase(iter);
                break;
            }
        }
    }
    void EventSystem::UnsubscribeAllEvents(IListener* listener)
    {
        std::multimap<EventID, IListener*>::iterator iter = listeners.begin();

        while (iter != listeners.end())
        {
            if ((*iter).second == listener) { listeners.erase(iter); return; }
            else iter++;
        }
    }

    void EventSystem::SendEvent(IEvent& e)
    {
        HandleEvent(e);
    }

    void EventSystem::ProcessEvents()
    {
        //TODO: event queue
    }

    void EventSystem::HandleEvent(IEvent& e)
    {
        std::pair<std::multimap<EventID, IListener*>::iterator, std::multimap<EventID, IListener*>::iterator> range;

        range = listeners.equal_range(e.GetEventID());

        for (std::multimap<EventID, IListener*>::iterator iter = range.first; iter != range.second; iter++)
        {
            bool handled = (*iter).second->OnEvent(e);
            if (handled) break;
        }
    }
}