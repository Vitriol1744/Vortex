/*
 * Created by v1tr10l7 on 24.10.2026.
 * Copyright (c) 2024-2026, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

import Prism.Utility.Delegate;

namespace Coral
{
    class ManagedAssembly;
};

namespace Vortex
{
    namespace ScriptEngine::ScriptGlue
    {

        using RegisterGlueFunction
            = Delegate<void(Coral::ManagedAssembly& coreAssembly)>;

        void RegisterAll(Coral::ManagedAssembly& coreAssembly);
    }; // namespace ScriptEngine::ScriptGlue
}; // namespace Vortex
