/*
 * Created by v1tr10l7 on 18.11.2026.
 * Copyright (c) 2024-2026, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#include <Vortex/Physics/PhysXInternal.hpp>

#include <Vortex/Core/Assertions.hpp>
#include <Vortex/Core/Core.hpp>

import Vortex.Physics;

using namespace physx;

namespace Vortex::PhysicsEngine
{
    namespace
    {
        class ErrorCallback final : public PxErrorCallback
        {
          public:
            void reportError(PxErrorCode::Enum code, const char* message,
                             const char* file, int line) override
            {
                StringView text  = message ? StringView(message) : StringView();
                StringView where = file ? StringView(file) : StringView();
                switch (code)
                {
                    case PxErrorCode::eDEBUG_INFO:
                        VtCoreTraceTag("Physics", "{} ({}:{})", text, where, line);
                        break;
                    case PxErrorCode::eDEBUG_WARNING:
                    case PxErrorCode::ePERF_WARNING:
                        VtCoreWarnTag("Physics", "{} ({}:{})", text, where, line);
                        break;
                    default:
                        VtCoreErrorTag("Physics", "{} ({}:{})", text, where, line);
                        break;
                }
            }
        };

        constexpr u32         kWorkerThreads = 2;

        PxDefaultAllocator    s_Allocator;
        ErrorCallback         s_ErrorCallback;
        PxFoundation*         s_Foundation = nullptr;
        PxPhysics*            s_Physics    = nullptr;
        PxDefaultCpuDispatcher* s_Dispatcher = nullptr;
        PhysicsSettings       s_Settings;
    } // namespace

    bool Initialize()
    {
        if (s_Physics) return true;

        s_Foundation = PxCreateFoundation(PX_PHYSICS_VERSION, s_Allocator,
                                          s_ErrorCallback);
        if (!s_Foundation)
        {
            VtCoreErrorTag("Physics", "PxCreateFoundation failed");
            return false;
        }

        s_Physics = PxCreatePhysics(PX_PHYSICS_VERSION, *s_Foundation,
                                    PxTolerancesScale(), false, nullptr);
        if (!s_Physics)
        {
            VtCoreErrorTag("Physics", "PxCreatePhysics failed");
            Shutdown();
            return false;
        }

        s_Dispatcher = PxDefaultCpuDispatcherCreate(kWorkerThreads);
        if (!s_Dispatcher)
        {
            VtCoreErrorTag("Physics", "Can't create the CPU dispatcher");
            Shutdown();
            return false;
        }

        VtCoreInfoTag("Physics", "PhysX {}.{}.{} initialized",
                      u32(PX_PHYSICS_VERSION_MAJOR), u32(PX_PHYSICS_VERSION_MINOR),
                      u32(PX_PHYSICS_VERSION_BUGFIX));
        return true;
    }

    void Shutdown()
    {
        if (s_Dispatcher) s_Dispatcher->release();
        if (s_Physics) s_Physics->release();
        if (s_Foundation) s_Foundation->release();
        s_Dispatcher = nullptr;
        s_Physics    = nullptr;
        s_Foundation = nullptr;
    }

    bool             IsInitialized() { return s_Physics != nullptr; }
    PhysicsSettings& Settings() { return s_Settings; }

    namespace Internal
    {
        PxPhysics& Physics()
        {
            VtCoreAssertMsg(s_Physics, "PhysicsEngine isn't initialized");
            return *s_Physics;
        }
        PxDefaultCpuDispatcher& Dispatcher()
        {
            VtCoreAssertMsg(s_Dispatcher, "PhysicsEngine isn't initialized");
            return *s_Dispatcher;
        }
    } // namespace Internal
} // namespace Vortex::PhysicsEngine
