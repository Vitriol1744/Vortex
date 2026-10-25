/*
 * Created by v1tr10l7 on 24.05.2026.
 * Copyright (c) 2024-2026, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include <Prism/String/String.hpp>
#include <Vortex/Script/Export.hpp>

namespace Vortex
{
    namespace ScriptEngine
    {
        void VT_SCRIPT_API Initialize();
        void VT_SCRIPT_API Shutdown();

        bool VT_SCRIPT_API ReloadAssembly();

        void VT_SCRIPT_API AddInternalCall(StringView internalClass,
                                           StringView internalCall, void* func);
        template <typename F>
        void AddInternalCall(StringView internalClass, StringView internalCall,
                             F func)
        {
            return AddInternalCall(internalClass, internalCall,
                                   reinterpret_cast<void*>(func));
        }

        void VT_SCRIPT_API OnCreateEntity(u64 entityID, StringView className);
        void VT_SCRIPT_API OnUpdateEntity(u64 entityID, Timestep ts);
        void VT_SCRIPT_API OnUpdateRuntime(float deltaTime);
        void VT_SCRIPT_API OnDestroyEntity(u64 entityID);
        void VT_SCRIPT_API ClearScriptInstances();
    }; // namespace ScriptEngine
}; // namespace Vortex
