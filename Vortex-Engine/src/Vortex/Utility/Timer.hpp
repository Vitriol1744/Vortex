//
// Created by vitriol1744 on 21.07.2021.
//
#pragma once

#include "Vortex/Core/Core.hpp"

#include "Vortex/Core/Time.hpp"
#include "Vortex/Core/Timestep.hpp"

namespace Vortex::Utility
{
    class VT_API Timer
    {
        public:
            Timer() { Start(); }

            inline void Start() { start = Time::GetTime(); }

            //TODO: Add Operators overloading for Timestep!
            inline Timestep Elapsed() const { return Timestep(Time::GetTime().Seconds() - start.Seconds()); }

        private:
            Timestep start;
    };
}

