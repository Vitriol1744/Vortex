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
        EventQueue(usize size = 64_kib)
            : m_Allocator(
                  [](usize sz)
                  {
                      auto range = MemoryManager::ReserveVirtualRange(sz);
                      MemoryManager::CommitVirtualRange(
                          range.Raw(), range.Size(),
                          MemoryProtection::eReadWrite);
                      return range;
                  }(size))
        {
        }

        ~EventQueue()
        {
            MemoryManager::ReleaseVirtualRange(m_Allocator.Range());
        }

        template <typename T, typename... Args>
        void Push(Args&&... args)
        {
            constexpr usize alignment   = Max(alignof(T), alignof(EventHeader));
            constexpr usize headerSize  = sizeof(EventHeader);

            usize           startOffset = m_Allocator.CurrentOffset();
            constexpr usize payloadSize = sizeof(T);
            constexpr usize totalSize   = headerSize + payloadSize;

            u8* allocationBlock = m_Allocator.Allocate(totalSize, alignment);
            if (!allocationBlock) return;

            usize        endOffset = m_Allocator.CurrentOffset();

            EventHeader* header
                = reinterpret_cast<EventHeader*>(allocationBlock);
            header->ID           = EventMetadata<T>::ID();
            header->Size         = payloadSize;
            header->TotalStride  = static_cast<u32>(endOffset - startOffset);

            void* payloadAddress = allocationBlock + headerSize;
            new (payloadAddress) T(Forward<Args>(args)...);
        }
        void  Clear() { m_Allocator.Reset(); }
        u8*   Data() { return m_Allocator.Raw(); }
        usize Size() { return m_Allocator.CurrentOffset(); }

      private:
        ArenaAllocator m_Allocator;
    };
}; // namespace Vortex
