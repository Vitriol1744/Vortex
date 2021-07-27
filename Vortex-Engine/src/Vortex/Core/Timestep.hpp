//
// Created by vitriol1744 on 21.07.2021.
//
#pragma once

#include "Vortex/Core/Core.hpp"

namespace Vortex
{
    class VT_API Timestep
    {
        public:
            inline Timestep() = default;
            inline Timestep(float64 seconds)
            {
                if (seconds < 0.0) this->seconds = 0.0;
                else this->seconds = seconds;
            }

            inline float64 Seconds() const { return seconds; }
            inline float64 Milliseconds() const { return Seconds() * 1000; }
            inline float64 Microseconds() const { return Milliseconds() * 1000; }
            inline float64 Nanoseconds() const { return Microseconds() * 1000; }

            inline void Seconds(float64 seconds) { this->seconds = seconds; }

        private:
            float64 seconds;
    };
}

