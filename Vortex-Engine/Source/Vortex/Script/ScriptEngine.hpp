/*
 * Created by v1tr10l7 on 24.05.2026.
 * Copyright (c) 2024-2026, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include <Vortex/Core/Types.hpp>
#include <Vortex/Script/Export.hpp>

import Prism.Containers.Vector;
import Prism.String.String;
import Prism.String.StringView;

namespace Vortex
{
    struct ScriptComponent;
    // Components.hpp
    enum class ScriptFieldType : u8;

    /// A public field of a script class, as the editor shows it.
    struct ScriptFieldInfo
    {
        String          Name{};
        /// Managed type name ("System.Single", "Vortex.Vector3"...).
        String          TypeName{};
        /// eNone for types the editor can't edit (shown read-only).
        ScriptFieldType Type{};
        /// The value a new instance starts with (ScriptFieldSize bytes;
        /// strings in DefaultText).
        alignas(8) u8   Default[16]{};
        String          DefaultText{};
    };

    /// A Vortex.Actor subclass of the app assembly.
    struct ScriptClassInfo
    {
        String                  FullName{};
        /// Namespace and class name, split at the last dot.
        String                  Namespace{};
        String                  Name{};
        /// Full name of the base class ("Vortex.Actor" or another script).
        String                  BaseName{};
        Vector<ScriptFieldInfo> Fields;
    };

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
        /// Creates the actor's instance with the component's edited fields
        /// applied before OnCreate.
        void VT_SCRIPT_API OnCreateEntity(u64 entityID, const ScriptComponent& script);
        void VT_SCRIPT_API OnUpdateEntity(u64 entityID, Timestep ts);
        void VT_SCRIPT_API OnUpdateRuntime(float deltaTime);
        void VT_SCRIPT_API OnDestroyEntity(u64 entityID);
        void VT_SCRIPT_API ClearScriptInstances();

        // Reflection, for the editor ---------------------------------------

        /// Whether the runtime and the app assembly are loaded.
        bool VT_SCRIPT_API       IsLoaded();
        /// Path of the app assembly (the game's scripts).
        StringView VT_SCRIPT_API AppAssemblyPath();
        /// Changes on every (re)load, so callers can drop cached data.
        u64 VT_SCRIPT_API        Generation();
        /// Every concrete Vortex.Actor subclass, sorted by name. Built on
        /// first use after a load (instantiates each class once to read its
        /// field defaults).
        const Vector<ScriptClassInfo>& VT_SCRIPT_API Classes();
        /// Null if the app assembly has no such script class.
        const ScriptClassInfo* VT_SCRIPT_API FindClass(StringView fullName);

        /// Whether `entityID` has a live instance (Play mode).
        bool VT_SCRIPT_API HasInstance(u64 entityID);
        /// Reads or writes a field of the live instance; false without one or
        /// for a field type the editor can't handle. `value` holds
        /// ScriptFieldSize(field.Type) bytes; strings use `text`.
        bool VT_SCRIPT_API GetInstanceField(u64 entityID, const ScriptFieldInfo& field,
                                            void* value, String* text = nullptr);
        bool VT_SCRIPT_API SetInstanceField(u64 entityID, const ScriptFieldInfo& field,
                                            const void* value, StringView text = {});
    }; // namespace ScriptEngine
}; // namespace Vortex
