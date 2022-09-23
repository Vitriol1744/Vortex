//
// Created by Vitriol1744 on 08.09.2022.
//

#ifndef VORTEX_ENGINE_TIMER_HPP
#define VORTEX_ENGINE_TIMER_HPP
#include "Vortex/Core/Core.hpp"

#include "Vortex/Core/Time.hpp"
#include "Vortex/Core/Timestep.hpp"

namespace Vortex
{
    class VT_API Timer
    {
        public:
            Timer() { Start(); }

            //TODO: Core: Time::GetTime();
            inline void Restart() { /*start = Time::GetTime();*/}

            inline Timestep Elapsed() const { return /*Time::GetTime() -*/ start; }

        private:
            Timestep start;
    };
}

#endif //VORTEX_ENGINE_TIMER_HPP
