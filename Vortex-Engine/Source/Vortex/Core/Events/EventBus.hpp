/*
 * Created by v1tr10l7 on 21.05.2026.
 * Copyright (c) 2024-2026, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include <Prism/Memory/Scope.hpp>
#include <Vortex/Core/Events/EventQueue.hpp>

namespace Vortex
{
    class IEventWrapper
    {
      public:
        virtual ~IEventWrapper()          = default;
        virtual void Dispatch(void* data) = 0;
    };

    template <typename T>
    class EventWrapper : public IEventWrapper
    {
      public:
        using Callback = std::function<void(const T&)>;

        void AddListener(Callback cb) { m_Callbacks.PushBack(cb); }

        void Dispatch(void* data) override
        {
            T* eventData = static_cast<T*>(data);
            for (auto& cb : m_Callbacks) cb(*eventData);
        }

      private:
        Vector<Callback> m_Callbacks;
    };
    class EventBus
    {
      public:
        template <typename T, typename... Args>
        void PublishImmediate(Args&&... args)
        {
            EventID id = EventMetadata<T>::ID;

            T       eventInstance(Forward<Args>(args)...);

            if (id < m_Wrappers.Size() && m_Wrappers[id])
                m_Wrappers[id]->Dispatch(&eventInstance);
        }

        template <typename T, typename... Args>
        void PublishDelayed(Args&&... args)
        {
            // Enforce safe memory boundaries or direct to specific queues based
            // on category
            static_assert(EventMetadata<T>::Category != EventCategory::eSystem,
                          "Critical System Events should generally not be "
                          "deferred to minimize lag.");

            m_ActiveQueue->Push<T>(Forward<Args>(args)...);
        }

        template <typename T>
        void Subscribe(typename EventWrapper<T>::Callback cb)
        {
            EventID id = EventMetadata<T>::ID;
            if (m_Wrappers.Size() <= id) m_Wrappers.Resize(id + 1);
            if (!m_Wrappers[id])
                m_Wrappers[id] = CreateScope<EventWrapper<T>>();

            static_cast<EventWrapper<T>*>(m_Wrappers[id].Raw())
                ->AddListener(cb);
        }

        void DispatchDelayed()
        {
            auto* queueToProcess = m_ActiveQueue;
            m_ActiveQueue
                = (m_ActiveQueue == &m_QueueA) ? &m_QueueB : &m_QueueA;

            usize   readOffset = 0;
            u8* data       = queueToProcess->Data();
            usize   totalSize  = queueToProcess->Size();

            while (readOffset < totalSize)
            {
                EventHeader* header
                    = reinterpret_cast<EventHeader*>(data + readOffset);
                readOffset += sizeof(EventHeader);

                void* eventData = data + readOffset;
                if (header->ID < m_Wrappers.Size() && m_Wrappers[header->ID])
                    m_Wrappers[header->ID]->Dispatch(eventData);

                readOffset += header->Size;
            }
            queueToProcess->Clear();
        }

      private:
        EventQueue                   m_QueueA;
        EventQueue                   m_QueueB;
        EventQueue*                  m_ActiveQueue = &m_QueueA;
        Vector<Scope<IEventWrapper>> m_Wrappers;
    };
}; // namespace Vortex
