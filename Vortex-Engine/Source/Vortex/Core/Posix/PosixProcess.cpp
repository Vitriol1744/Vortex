/*
 * Created by v1tr10l7 on 14.09.2026.
 * Copyright (c) 2024-2026, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#ifndef VT_PLATFORM_ANDROID
    #include <Vortex/Core/Posix/PosixProcess.hpp>
#else
    #include <Vortex/Core/Process.hpp>
#endif

namespace Vortex
{
    Scope<Process> Process::Create(String command, Vector<String> args)
    {
#ifndef VT_PLATFORM_ANDROID
        return CreateScope<PosixProcess>(Move(command), Move(args));
#else
        return nullptr;
#endif
    }
}; // namespace Vortex
