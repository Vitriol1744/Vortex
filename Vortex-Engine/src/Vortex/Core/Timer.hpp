/*
 * Created by v1tr10l7 on 21.08.2024.
 * Copyright (c) 2024-2024, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include "Vortex/Core/Time.hpp"

namespace Vortex
{
    class Timer
    {
      public:
        inline Timer() { Start(); }

        inline void     Start() { m_Start = Time::GetCurrentTime(); }
        inline Timestep Restart()
        {
            Timestep elapsedTime = ElapsedTime();
            m_Start              = Time::GetCurrentTime();

            return elapsedTime;
        }
        inline Timestep ElapsedTime() const
        {
            return Time::GetCurrentTime() - m_Start;
        }

      private:
        Timestep m_Start{};
    };
}; // namespace Vortex
