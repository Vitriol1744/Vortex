//
// Created by Vitriol1744 on 08.09.2022.
//
#ifndef VORTEX_LISTENER_HPP
#define VORTEX_LISTENER_HPP

#include "Vortex/Core/Export.hpp"

namespace Vortex
{
    struct IEvent;

    class VT_API IListener
    {
        public:
            IListener() = default;
            virtual ~IListener() = default;

            virtual bool OnEvent(IEvent& event) = 0;
    };
}

#endif //VORTEX_LISTENER_HPP
