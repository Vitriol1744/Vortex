//
// Created by vitriol1744 on 21.07.2021.
//
#pragma once

#include "Vortex/Core/Export.hpp"
#include "Vortex/Core/Types.hpp"

namespace Vortex
{
    constexpr float64 Milliseconds(float64 ms)
    {
        return ms / 1000;
    }
    constexpr float64 Microseconds(float64 us)
    {
        return Milliseconds(us) / 1000;
    }
    constexpr float64 Nanoseconds(float ns)
    {
        return Microseconds(ns) / 1000;
    }

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

