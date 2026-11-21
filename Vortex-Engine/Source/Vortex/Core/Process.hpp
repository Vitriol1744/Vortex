/*
 * Created by v1tr10l7 on 14.09.2026.
 * Copyright (c) 2024-2026, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

import Prism.Containers.Vector;
import Prism.Memory.Scope;
import Prism.String.String;

#include <Vortex/Core/Export.hpp>
#include <Vortex/Core/Types.hpp>

namespace Vortex
{
    enum class ProcessState
    {
        eReady,
        eRunning,
        eExited,
        eFailed
    };

    struct ProcessResult
    {
        i32    exitCode{-1};
        String stdoutOutput;
        String stderrOutput;
    };

    class VT_CORE_API Process
    {
      public:
        virtual ~Process() = default;

        static Scope<Process> Create(String command, Vector<String> args);

        virtual bool          Start() = 0;
        virtual ProcessState  Poll()  = 0;
        virtual ProcessResult Wait()  = 0;
        virtual void          Kill()  = 0;

        PM_NODISCARD
        virtual ProcessState State() const = 0;
    };
}; // namespace Vortex
