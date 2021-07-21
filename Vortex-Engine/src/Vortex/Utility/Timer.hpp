//
// Created by vitriol1744 on 21.07.2021.
//
#pragma once

#include "Core/Core.hpp"

#include "Core/Time.hpp"
#include "Core/Timestep.hpp"

namespace Vortex::Utility
{
    class VT_API Timer
    {
        public:
            inline Timer() { Start(); }

            inline void Start() { start = Time::GetTime(); }

            //TODO: Add Operators overloading for Timestep!
            inline Timestep Elapsed() const { return Timestep(Time::GetTime().Seconds() - start.Seconds()); }

        private:
            Timestep start;
    };
}

