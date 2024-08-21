/*
 * Created by v1tr10l7 on 21.08.2024.
 * Copyright (c) 2024-2024, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include "Vortex/Core/Types.hpp"

namespace Vortex
{
    class Timestep
    {
      public:
        inline Timestep() = default;
        inline Timestep(f64 seconds)
            : m_Seconds(seconds)
        {
            m_Seconds = std::max(seconds, 0.0);
        }

        inline      operator f64() const { return m_Seconds; }

        inline f64  Seconds() const { return m_Seconds; }
        inline f64  Milliseconds() const { return Seconds() * 1000; }
        inline f64  Microseconds() const { return Milliseconds() * 1000; }
        inline f64  Nanoseconds() const { return Microseconds() * 1000; }

        inline void Set(f64 seconds) { m_Seconds = std::max(seconds, 0.0); }

        inline auto operator<=>(const Timestep& other) const = default;

      private:
        f64 m_Seconds = 0;
    };
}; // namespace Vortex
