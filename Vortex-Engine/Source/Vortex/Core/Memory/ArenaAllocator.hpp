/*
 * Created by v1tr10l7 on 14.06.2026.
 * Copyright (c) 2024-2026, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include <Prism/Containers/Span.hpp>
#include <Prism/Memory/Pointer.hpp>

namespace Vortex
{
    class ArenaAllocator
    {
      public:
        explicit ArenaAllocator(Span<u8> memoryBuffer)
            : m_Buffer(memoryBuffer)
            , m_Offset(0)
        {
        }

        ArenaAllocator(const ArenaAllocator&)             = delete;
        ArenaAllocator&  operator=(const ArenaAllocator&) = delete;

        PM_NODISCARD u8* Allocate(usize size, usize alignment) PM_NOEXCEPT
        {
            upointer currentAddress
                = reinterpret_cast<upointer>(m_Buffer.Raw()) + m_Offset;

            usize padding
                = (alignment - (currentAddress % alignment)) % alignment;

            if (m_Offset + padding + size > m_Buffer.Size()) return nullptr;

            m_Offset += padding;
            u8* ptr = m_Buffer.Raw() + m_Offset;
            m_Offset += size;

            return ptr;
        }

        void         Reset() PM_NOEXCEPT { m_Offset = 0; }

        PM_NODISCARD Span<u8> Range() const { return m_Buffer; }
        PM_NODISCARD Pointer  Raw() const { return m_Buffer.Raw(); }
        PM_NODISCARD usize    Size() const { return m_Buffer.Size(); }
        PM_NODISCARD
        usize CurrentOffset() const PM_NOEXCEPT { return m_Offset; }

      private:
        Span<u8> m_Buffer;
        usize    m_Offset;
    };
}; // namespace Vortex
