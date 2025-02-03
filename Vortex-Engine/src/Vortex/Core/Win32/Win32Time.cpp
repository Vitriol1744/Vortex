/*
 * Created by v1tr10l7 on 21.08.2024.
 * Copyright (c) 2024-2024, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#include "vtpch.hpp"

#include "Vortex/Core/Time.hpp"

#include "Vortex/Core/Win32/Windows.hpp"

namespace Vortex
{
    namespace Time
    {
        static LARGE_INTEGER GetPerformanceFrequency()
        {
            LARGE_INTEGER frequency{};
            QueryPerformanceFrequency(&frequency);

            return frequency;
        }

        Timestep GetCurrentTime()
        {
            LARGE_INTEGER        performanceCounter{};
            static LARGE_INTEGER performanceFrequency
                = GetPerformanceFrequency();

            QueryPerformanceCounter(&performanceCounter);
            return static_cast<f64>(performanceCounter.QuadPart)
                 / static_cast<f64>(performanceFrequency.QuadPart);
        }

        void Sleep(Timestep timestep)
        {
            ::Sleep(static_cast<u64>(timestep.Milliseconds()));
        }
    } // namespace Time
};    // namespace Vortex
