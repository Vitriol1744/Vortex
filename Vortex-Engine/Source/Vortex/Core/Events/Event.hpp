/*
 * Created by v1tr10l7 on 18.07.2024.
 * Copyright (c) 2024-2024, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include <Prism/Algorithm/Find.hpp>
#include <Prism/Containers/Queue.hpp>
#include <Prism/Containers/Vector.hpp>

#include <Vortex/Core/Assertions.hpp>

namespace Vortex
{
    using EventID = u64;
    struct VT_CORE_API EventType
    {
        static EventID NextID()
        {
            static EventID id = 0;

            return ++id;
        }
    };

    enum class EventCategory : u8
    {
        // Core OS engine state (Engine Shutdown, Module Loading)
        eSystem,
        // OS Native UI interactions (WindowResized, FocusLost, Closed)
        eWindow,
        // High-frequency hardware notifications (KeyPressed, MouseMoved)
        eInput,
        // Fixed-update physics triggers (CollisionStart,TriggerEnter)
        ePhysics,
        // Audio execution cues (PlaySound3D, MixGroupChanged)
        eAudio,
        // Network synchronization payloads (PacketReceived, ClientDisconnected)
        eNetwork,
        // Frontend render commands (ViewportChanged, SwapchainRecreated)
        eGraphics,
        // Editor Specific events
        eEditor,
        // Contextual gameplay logic components (PlayerScore, InventoryItemUsed)
        eUser

    };
    template <typename T>
    concept IsEvent = requires {
        { T::Category } -> SameAs<const EventCategory&>;
    };

    constexpr u64 HashString(const char* str)
    {
        u64 hash = 14695981039346656037ULL;
        while (*str)
        {
            hash ^= static_cast<u64>(*str++);
            hash *= 1099511628211ULL;
        }
        return hash;
    }

    template <typename T>
    struct EventMetadata
    {
        static constexpr EventCategory Category = T::Category;

        static EventID                 ID() noexcept
        {
            // Guaranteed identical across all dynamic libraries and TUs
            static const EventID id = HashString(typeid(T).name());
            return id;
        }
    };
    struct EventHeader
    {
        EventID ID;
        u32     Size;
        u32     TotalStride;
    };
} // namespace Vortex
