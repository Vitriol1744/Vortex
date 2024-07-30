/*
 * Created by v1tr10l7 on 18.07.2024.
 * Copyright (c) 2024-2024, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include "Vortex/Core/Assertions.hpp"
#include "Vortex/Core/Events/EventSystem.hpp"

namespace Vortex
{
    class EventBase
    {
      public:
        virtual ~EventBase()    = default;

        virtual void Dispatch() = 0;
    };

    template <typename... Args>
    class Event : public EventBase
    {
      public:
        using EventListener = std::function<bool()>;

        inline void RemoveAllListeners() { m_Listeners.clear(); }
        [[nodiscard]]
        inline int GetListenersCount() const
        {
            return m_Listeners.size();
        }

        inline void AddListener(EventListener listener)
        {
            m_Listeners.push_back(listener);
        }
        inline void operator+=(EventListener listener)
        {
            AddListener(listener);
        }
        inline void operator()() { EventSystem::PushEvent(this); }
        inline void Dispatch() override
        {
            for (const auto& listener : m_Listeners)
                if (listener && listener()) break;
        }

      private:
        std::vector<EventListener> m_Listeners;
    };

    template <typename Arg1>
    class Event<Arg1> : public EventBase
    {
      public:
        using EventListener = std::function<bool(Arg1)>;

        inline void RemoveAllListeners() { m_Listeners.clear(); }
        [[nodiscard]]
        inline int GetListenersCount() const
        {
            return m_Listeners.size();
        }

        inline void AddListener(EventListener listener)
        {
            m_Listeners.push_back(listener);
        }
        inline void operator+=(EventListener listener)
        {
            AddListener(listener);
        }
        inline void operator()(Arg1 arg1)
        {
            m_EventData.push(arg1);
            EventSystem::PushEvent(this);
        }
        inline void Dispatch() override
        {
            Arg1& arg1 = m_EventData.front();
            for (const auto& listener : m_Listeners)
                if (listener && listener(arg1)) break;
            m_EventData.pop();
        }

      private:
        std::vector<std::function<bool(Arg1)>> m_Listeners;
        std::queue<Arg1>                       m_EventData;
    };

    template <typename Arg1, typename Arg2>
    class Event<Arg1, Arg2> : public EventBase
    {
      public:
        using EventListener = std::function<bool(Arg1, Arg2)>;

        inline void RemoveAllListeners() { m_Listeners.clear(); }
        [[nodiscard]]
        inline int GetListenersCount() const
        {
            return m_Listeners;
        }

        inline void AddListener(const EventListener& listener)
        {
            m_Listeners.push_back(listener);
        }
        inline void operator+=(const EventListener& listener)
        {
            AddListener(listener);
        }
        inline void operator()(Arg1 arg1, Arg2 arg2)
        {
            m_EventData.push({arg1, arg2});
            EventSystem::PushEvent(this);
        }
        inline void Dispatch() override
        {
            auto& [arg1, arg2] = m_EventData.front();
            for (const auto& listener : m_Listeners)
                if (listener && listener(arg1, arg2)) break;
            m_EventData.pop();
        }

      private:
        std::vector<EventListener> m_Listeners;
        struct EventData
        {
            Arg1 arg1;
            Arg2 arg2;
        };
        std::queue<EventData> m_EventData;
    };
    template <typename Arg1, typename Arg2, typename Arg3>
    class Event<Arg1, Arg2, Arg3> : public EventBase
    {
      public:
        using EventListener = std::function<bool(Arg1, Arg2, Arg3)>;

        inline void RemoveAllListeners() { m_Listeners.clear(); }
        [[nodiscard]]
        inline int GetListenersCount() const
        {
            return m_Listeners.size();
        }

        inline void AddListener(const EventListener& listener)
        {
            m_Listeners.push_back(listener);
        }
        inline void operator+=(EventListener listener)
        {
            m_Listeners.push_back(listener);
        }
        inline void operator()(Arg1 arg1, Arg2 arg2, Arg3 arg3)
        {
            m_EventData.push({arg1, arg2, arg3});
            EventSystem::PushEvent(this);
        }
        inline void Dispatch() override
        {
            auto& [arg1, arg2, arg3] = m_EventData.front();
            for (const auto& listener : m_Listeners)
                if (listener && listener(arg1, arg2, arg3)) break;
            m_EventData.pop();
        }

      private:
        std::vector<EventListener> m_Listeners;
        struct EventData
        {
            Arg1 arg1;
            Arg2 arg2;
            Arg3 arg3;
        };
        std::queue<EventData> m_EventData;
    };
} // namespace Vortex
