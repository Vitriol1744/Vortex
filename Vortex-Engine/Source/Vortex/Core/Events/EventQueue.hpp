/*
 * Created by v1tr10l7 on 14.06.2026.
 * Copyright (c) 2024-2026, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include <Vortex/Core/Events/Event.hpp>
#include <Vortex/Core/Memory/ArenaAllocator.hpp>
#include <Vortex/Core/Memory/MemoryManager.hpp>

namespace Vortex
{
    class EventQueue
    {
      public:
        EventQueue(usize size = 1024 * 64)
            : m_Allocator(MemoryManager::ReserveVirtualRange(size))
        {
        }

        ~EventQueue()
        {
            MemoryManager::ReleaseVirtualRange(m_Allocator.Range());
        }

        template <typename T, typename... Args>
        void Push(Args&&... args)
        {
            u8* headerBytes = m_Allocator.Allocate(sizeof(EventHeader),
                                                   alignof(EventHeader));
            if (!headerBytes) return;

            u8* eventBytes = m_Allocator.Allocate(sizeof(T), alignof(T));
            if (!eventBytes) return;

            EventHeader* header = reinterpret_cast<EventHeader*>(headerBytes);
            header->ID          = EventMetadata<T>::ID;
            header->Size        = sizeof(T);

            new (static_cast<void*>(eventBytes)) T(Forward<Args>(args)...);
        }

        void  Clear() { m_Allocator.Reset(); }
        u8*   Data() { return m_Allocator.Raw(); }
        usize Size() { return m_Allocator.CurrentOffset(); }

      private:
        ArenaAllocator m_Allocator;
    };
}; // namespace Vortex
