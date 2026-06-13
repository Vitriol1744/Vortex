/*
 * Created by v1tr10l7 on 21.05.2026.
 * Copyright (c) 2024-2026, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

namespace Vortex
{
    class EventBus
    {
      public:
        template <typename T, typename... Args>
        inline void Publish(Args&&... args)
        {
            m_ActiveQueue.Push<T>(Forward<Args>(args)...);
        }
        template <typename T>
        inline void Subscribe(typename EventWrapper<T>::Callback callback)
        {
            EventID id = EventMetadata<T>::ID;
            if (m_Wrappers.Size() <= id) m_Wrappers.Resize(id + 1);
            if (!m_Wrappers[id]) m_Wrappers[id] = MakeUnique<EventWrapper<T>>();

            static_cast<EventWrapper<T>*>(m_Wrappers[id].Get())
                ->AddListener(callback);
        }

        inline void Dispatch()
        {
            auto* queueToProcess = m_ActiveQueue;
            m_ActiveQueue        = &m_InactiveQueue;
        }
    };
}; // namespace Vortex
