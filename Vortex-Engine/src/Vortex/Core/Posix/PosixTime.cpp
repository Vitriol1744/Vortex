/*
 * Created by v1tr10l7 on 21.08.2024.
 * Copyright (c) 2024-2024, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#include "vtpch.hpp"

#include "Vortex/Core/Time.hpp"

#include <unistd.h>

namespace Vortex
{
    namespace Time
    {
        Timestep GetCurrentTime()
        {
            timespec currentTime{};
            clock_gettime(CLOCK_MONOTONIC, &currentTime);

            u64 time = static_cast<u64>(currentTime.tv_sec) * 1000000ull
                     + currentTime.tv_nsec / 1000ull;

            return static_cast<f64>(time / 1000000.0);
        }

        void Sleep(Timestep timestep) { usleep(timestep.Microseconds()); }
    }; // namespace Time
};     // namespace Vortex
