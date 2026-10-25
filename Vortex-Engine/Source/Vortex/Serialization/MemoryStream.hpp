/*
 * Created by v1tr10l7 on 13.09.2026.
 * Copyright (c) 2024-2026, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include <Prism/Utility/Path.hpp>
#include <Vortex/Serialization/Stream.hpp>

namespace Vortex
{
    // Non-owning reader over raw memory buffer
    class MemoryStreamReader : public StreamReader
    {
      public:
        MemoryStreamReader(const void* buffer, usize size)
            : m_Buffer(reinterpret_cast<const char*>(buffer))
            , m_Size(size)
            , m_Position(0)
        {
        }

        virtual bool Good() const override
        {
            return m_Position <= m_Size && m_Buffer != nullptr;
        }

        virtual u64  Position() const override { return m_Position; }
        virtual bool SetPosition(u64 position) override
        {
            if (position > m_Size) return false;
            m_Position = position;
            return true;
        }

        virtual bool ReadBytes(char* destination, usize size) override
        {
            if (m_Position + size > m_Size) return false;

            Memory::Copy(destination, m_Buffer + m_Position, size);
            m_Position += size;
            return true;
        }

        usize TotalSize() const { return m_Size; }

      private:
        const char* m_Buffer;
        usize       m_Size;
        usize       m_Position;
    };

    class MemoryStreamWriter : public StreamWriter
    {
      public:
        MemoryStreamWriter()
            : m_IsFixed(false)
            , m_FixedBuffer(nullptr)
            , m_FixedCapacity(0)
            , m_Position(0)
        {
        }

        // Non-owning fixed size buffer writer
        MemoryStreamWriter(void* buffer, usize capacity)
            : m_IsFixed(true)
            , m_FixedBuffer(reinterpret_cast<char*>(buffer))
            , m_FixedCapacity(capacity)
            , m_Position(0)
        {
        }

        virtual bool Good() const override
        {
            return m_IsFixed ? (m_Position <= m_FixedCapacity) : true;
        }

        virtual u64  Position() const override { return m_Position; }
        virtual bool SetPosition(u64 position) override
        {
            if (m_IsFixed && position > m_FixedCapacity) return false;

            if (!m_IsFixed && position > m_DynamicBuffer.Size())
                m_DynamicBuffer.Resize(position);

            m_Position = position;
            return true;
        }

        virtual bool WriteBytes(const char* data, usize size) override
        {
            if (m_IsFixed)
            {
                if (m_Position + size > m_FixedCapacity) return false;

                Memory::Copy(m_FixedBuffer + m_Position, data, size);
            }
            else {
                if (m_Position + size > m_DynamicBuffer.Size())
                    m_DynamicBuffer.Resize(m_Position + size);

                Memory::Copy(m_DynamicBuffer.Raw() + m_Position, data, size);
            }

            m_Position += size;
            return true;
        }

        const char* Raw() const
        {
            return m_IsFixed ? m_FixedBuffer : m_DynamicBuffer.Raw();
        }

        usize Size() const { return m_Position; }

      private:
        bool         m_IsFixed;
        char*        m_FixedBuffer;
        usize        m_FixedCapacity;

        Vector<char> m_DynamicBuffer;
        usize        m_Position;
    };
} // namespace Vortex
