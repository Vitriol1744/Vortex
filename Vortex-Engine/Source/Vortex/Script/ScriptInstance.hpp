/*
 * Created by v1tr10l7 on 13.09.2026.
 * Copyright (c) 2024-2026, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include <Vortex/Core/Core.hpp>

#include <Coral/ManagedObject.hpp>
#include <Coral/Type.hpp>

namespace Vortex
{
    class ScriptInstance : public RefCounted<ScriptInstance>
    {
      public:
        inline ScriptInstance(Coral::Type& type, u64 entityID)
        {
            m_Instance = type.CreateInstance(entityID);
        }

        template <typename... Args>
        inline void Invoke(StringView methodName, Args&&... args)
        {
            return m_Instance.InvokeMethod(methodName.Raw(),
                                           Forward<Args>(args)...);
        }

        void InvokeOnCreate() { Invoke("OnCreate"); }
        void InvokeOnUpdate(float ts) { Invoke("OnUpdate", ts); }
        void InvokeOnDestroy() { Invoke("OnDestroy"); }

        Coral::ManagedObject& ManagedObject() { return m_Instance; }

      private:
        Coral::ManagedObject m_Instance;
    };
}; // namespace Vortex
