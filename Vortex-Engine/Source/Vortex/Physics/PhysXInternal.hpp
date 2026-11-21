/*
 * Created by v1tr10l7 on 18.11.2026.
 * Copyright (c) 2024-2026, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

// Private to Vortex-Physics: the PhysX objects shared by the engine and its
// scenes. A header rather than a module so that PhysX's headers never end up
// in a module's global fragment.

#include <PxPhysicsAPI.h>

namespace Vortex::PhysicsEngine::Internal
{
    physx::PxPhysics&             Physics();
    physx::PxDefaultCpuDispatcher& Dispatcher();
} // namespace Vortex::PhysicsEngine::Internal
