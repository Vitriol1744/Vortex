//
// Created by Vitriol1744 on 07.09.2022.
//
#ifndef VORTEX_TIMESTEP_HPP
#define VORTEX_TIMESTEP_HPP

#include "Vortex/Core/Export.hpp"
#include "Vortex/Core/Types.hpp"

namespace Vortex
{
    inline constexpr float64 Milliseconds(float64 ms) noexcept { return ms / 1000; }
    inline constexpr float64 Microseconds(float64 us) noexcept { return Milliseconds(us) / 1000; }
    inline constexpr float64 Nanoseconds(float64 ns) noexcept { return Microseconds(ns) / 1000; }

    class Timestep
    {
        public:
            inline Timestep() = default;
            inline Timestep(float64 seconds)
            {
                if (seconds < 0.0f) seconds = 0.0f;
                this->seconds = seconds;
            }

            inline float64 Seconds() noexcept { return seconds; }
            inline float64 Milliseconds() noexcept { return seconds * 1000; }
            inline float64 Microseconds() noexcept { return Milliseconds() * 1000; }
            inline float64 Nanoseconds() noexcept { return Microseconds() * 1000; }

            inline void Set(float64 seconds) noexcept { this->seconds = seconds; }

            friend Timestep operator+(Timestep lhs, Timestep rhs)
            {
                return Timestep(lhs.seconds + rhs.seconds);
            }
            friend Timestep operator-(Timestep lhs, Timestep rhs)
            {
                return Timestep(lhs.seconds - rhs.seconds);
            }

        private:
            float64 seconds;
    };
}

#endif //VORTEX_TIMESTEP_HPP
