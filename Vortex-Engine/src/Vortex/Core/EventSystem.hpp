//
// Created by vitriol1744 on 08.07.2021.
//
#pragma once

#include "Vortex/Core/Core.hpp"
#include <queue>

namespace Vortex
{
    class IEventBase;

    class VT_API EventSystem : public Singleton<EventSystem>
    {
        public:
            void PushEvent(IEventBase* event);

            void Update();

        private:
            std::queue<IEventBase*> eventBus;
    };
}