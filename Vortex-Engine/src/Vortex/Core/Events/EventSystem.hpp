//
// Created by Vitriol1744 on 08.09.2022.
//
#ifndef VORTEX_EVENTSYSTEM_HPP
#define VORTEX_EVENTSYSTEM_HPP

#include "Vortex/Core/Export.hpp"
#include "Vortex/Core/Types.hpp"
#include "Vortex/Core/Singleton.hpp"

#include "IEvent.hpp"
#include "IListener.hpp"

namespace Vortex
{
    class VT_API EventSystem : public Singleton<EventSystem>
    {
        public:
            void SubscribeEvent(EventID event_id, IListener* listener);
            void UnsubscribeEvent(EventID event_id, IListener* listener);
            void UnsubscribeAllEvents(IListener* listener);

            void SendEvent(IEvent& event);

            void ProcessEvents();

        private:
            std::multimap<EventID, IListener*> listeners;

            void HandleEvent(IEvent& e);
    };
}


#endif //VORTEX_EVENTSYSTEM_HPP
