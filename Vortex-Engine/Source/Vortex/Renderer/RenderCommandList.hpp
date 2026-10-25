/*
 * Created by v1tr10l7 on 13.07.2026.
 * Copyright (c) 2024-2026, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include <Vortex/Core/Core.hpp>

namespace Vortex
{
    class VT_API RenderCommandList
    {
      public:
        using ExecuteCommandFn
            = void (*)(void* payload, struct GraphicsContext& ctx);

        struct Header
        {
            ExecuteCommandFn Executor;
            Header*          Next;
        };

        RenderCommandList(u8* backingBuffer, usize capacity)
            : m_Buffer(backingBuffer)
            , m_Capacity(capacity)
            , m_Head(nullptr)
            , m_Tail(nullptr)
            , m_Offset(0)
        {
        }

        RenderCommandList(RenderCommandList&& other) noexcept = default;
        RenderCommandList& operator=(RenderCommandList&& other) noexcept
            = default;
        RenderCommandList(const RenderCommandList&)            = delete;
        RenderCommandList& operator=(const RenderCommandList&) = delete;

        ~RenderCommandList()                                   = default;

        template <typename T, typename... Args>
        void Record(Args&&... args)
        {
            usize totalSize = sizeof(Header) + sizeof(T);

            // Ensure accurate type-alignment constraints inside the raw buffer
            usize alignmentPaddiing
                = (alignof(T) - (m_Offset + sizeof(Header)) % alignof(T))
                % alignof(T);
            totalSize += alignmentPaddiing;

            if (m_Offset + totalSize > m_Capacity)
                // Error!
                return;

            u8* storagePointer = m_Buffer + m_Offset;
            m_Offset += totalSize;

            // Construct structural tracking header
            auto* header     = reinterpret_cast<Header*>(storagePointer);
            header->Next     = nullptr;
            header->Executor = [](void* payload, struct GraphicsContext& ctx)
            {
                T* concreteCommand = reinterpret_cast<T*>(payload);
                concreteCommand->Execute(ctx);
                concreteCommand->~T();
            };

            void* payloadPointer
                = storagePointer + sizeof(Header) + alignmentPaddiing;
            ::new (payloadPointer) T{Forward<Args>(args)...};

            if (!m_Head) m_Head = header;
            else m_Tail->Next = header;
            m_Tail = header;
        }

        Header* Root() const { return m_Head; }
        void    Clear()
        {
            m_Head   = nullptr;
            m_Tail   = nullptr;
            m_Offset = 0;
        }

      private:
        u8*     m_Buffer;
        usize   m_Capacity;
        Header* m_Head;
        Header* m_Tail;
        usize   m_Offset;
    };
}; // namespace Vortex
