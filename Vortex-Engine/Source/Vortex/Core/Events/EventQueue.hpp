/*
 * Created by v1tr10l7 on 14.06.2026.
 * Copyright (c) 2024-2026, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include <Vortex/Core/Events/Event.hpp>

namespace Vortex
{
    class EventQueue
    {
      public:
        EventQueue(usize size = 1024 * 64)
            : m_Capacity(size)
        {
            m_Data   = new u8[m_Capacity];
            m_Offset = 0;
        }

        ~EventQueue() { delete[] m_Data; }

        template <typename T, typename... Args>
        void Push(Args&&... args)
        {
            usize totalSize = sizeof(EventHeader) + sizeof(T);
            if (m_Offset + totalSize > m_Capacity)
                return; // Handle overflow properly in production

            // Write Header
            EventHeader* header
                = reinterpret_cast<EventHeader*>(m_Data + m_Offset);
            header->ID   = EventMetadata<T>::ID;
            header->Size = sizeof(T);
            m_Offset += sizeof(EventHeader);

            // Construct Event in-place (Placement New)
            new (m_Data + m_Offset) T(Forward<Args>(args)...);
            m_Offset += sizeof(T);
        }

        void  Clear() { m_Offset = 0; }
        u8*   Data() { return m_Data; }
        usize Size() { return m_Offset; }

      private:
        u8*   m_Data;
        usize m_Capacity;
        usize m_Offset;
    };
}; // namespace Vortex
