//
// Created by Vitriol1744 on 08.09.2022.
//
#ifndef VORTEX_EVENTS_HPP
#define VORTEX_EVENTS_HPP

namespace Vortex
{
    enum EventType
    {
        eKeyPressedEvent = 0,
        eKeyReleasedEvent,
        eKeyTypedEvent,
        eMonitorResolutionChangedEvent,
        eMonitorStateChangedEvent,
        eMouseButtonPressedEvent,
        eMouseButtonReleasedEvent,
        eMouseCursorEnterEvent,
        eMouseScrolledEvent,
        eMouseMovedEvent,
        eWindowClosedEvent,
        eWindowFocusChangedEvent,
        eWindowMovedEvent,
        eWindowResizedEvent,

        eEventTypesCount,
    };
}

#endif //VORTEX_EVENTS_HPP
