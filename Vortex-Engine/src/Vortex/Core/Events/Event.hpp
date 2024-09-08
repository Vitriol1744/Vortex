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
    template <typename T, typename... U>
    size_t GetFunctionAddress(std::function<T(U...)> f)
    {
        typedef T(fnType)(U...);
        fnType** fnPointer = f.template target<fnType*>();
        return (size_t)*fnPointer;
    }

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
        inline void operator-=(EventListener listener)
        {
            (void)std::remove_if(
                m_Listeners.begin(), m_Listeners.end(),
                [&listener](const EventListener& currentListener) {
                    return listener.target<bool()>()
                        == currentListener.target<bool()>();
                });
        }
        inline void operator()()
        {
            Dispatch();
            // EventSystem::PushEvent(this);
        }
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
        inline void operator-=(EventListener listener)
        {
            (void)std::remove_if(
                m_Listeners.begin(), m_Listeners.end(),
                [&listener](const EventListener& currentListener)
                {
                    return GetFunctionAddress(listener)
                        == GetFunctionAddress(currentListener);
                });
        }
        inline void operator()(Arg1 arg1)
        {
            for (const auto& listener : m_Listeners) listener(arg1);
            // m_EventData.push(arg1);
            // EventSystem::PushEvent(this);
        }
        inline void Dispatch() override {}

      private:
        std::vector<std::function<bool(Arg1)>> m_Listeners;
        [[maybe_unused]] std::queue<Arg1>      m_EventData;
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
        inline void operator-=(EventListener listener)
        {
            (void)std::remove_if(
                m_Listeners.begin(), m_Listeners.end(),
                [&listener](const EventListener& currentListener)
                {
                    return GetFunctionAddress(listener)
                        == GetFunctionAddress(currentListener);
                });
        }
        inline void operator()(Arg1 arg1, Arg2 arg2)
        {
            for (const auto& listener : m_Listeners) listener(arg1, arg2);
        }
        inline void Dispatch() override {}

      private:
        std::vector<EventListener> m_Listeners;
        struct EventData
        {
            Arg1 arg1;
            Arg2 arg2;
        };
        [[maybe_unused]] std::queue<EventData> m_EventData;
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
        inline void operator-=(EventListener listener)
        {
            (void)std::remove_if(
                m_Listeners.begin(), m_Listeners.end(),
                [&listener](const EventListener& currentListener)
                {
                    if (!listener || !currentListener) return false;
                    return GetFunctionAddress(listener)
                        == GetFunctionAddress(currentListener);
                });
        }
        inline void operator()(Arg1 arg1, Arg2 arg2, Arg3 arg3)
        {
            for (const auto& listener : m_Listeners) listener(arg1, arg2, arg3);
        }
        inline void Dispatch() override {}

      private:
        std::vector<EventListener> m_Listeners;
        struct EventData
        {
            Arg1 arg1;
            Arg2 arg2;
            Arg3 arg3;
        };
        [[maybe_unused]] std::queue<EventData> m_EventData;
    };
} // namespace Vortex
