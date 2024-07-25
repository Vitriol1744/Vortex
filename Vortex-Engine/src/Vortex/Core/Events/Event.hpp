/*
 * Created by v1tr10l7 on 18.07.2024.
 * Copyright (c) 2024-2024, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#define VtAddListener2(event, name, function, ...)                             \
    event.AddListener(std::to_string(name).data(),                             \
                      std::bind(function, __VA_ARGS__))
#define VtAddListener(event, function, ...)                                    \
    VtAddListener2(event, __COUNTER__, function, __VA_ARGS__)
#define VtPlaceholder(n) std::placeholders::_##n

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
        inline void RemoveAllListeners()
        {
            listeners.clear();
            listenersCount = 0;
        }
        [[nodiscard]]
        inline int GetListenersCount() const
        {
            return listenersCount;
        }

        inline void AddListener(const char*                  id,
                                const std::function<bool()>& listener)
        {
            if (listeners.find(id) == listeners.end())
            {
                listeners[id] = listener;
                listenersCount++;
            }
            else
                VtCoreAssertMsg(false,
                                "Listener with given id already exists!");
        }
        inline void RemoveListener(const char* id)
        {
            listeners.erase(id);
            listenersCount--;
        }
        inline void operator()() { EventSystem::PushEvent(this); }
        inline void Dispatch() override
        {
            for (const auto& listener : listeners)
                if (listener.second) listener.second.operator()();
        }

      private:
        int listenersCount = 0;
        std::unordered_map<const char*, std::function<bool()>> listeners;
    };

    template <typename Arg1>
    class Event<Arg1> : public EventBase
    {
      public:
        inline void RemoveAllListeners()
        {
            listeners.clear();
            listenersCount = 0;
        }
        [[nodiscard]]
        inline int GetListenersCount() const
        {
            return listenersCount;
        }

        inline void AddListener(const char*                      id,
                                const std::function<bool(Arg1)>& listener)
        {
            if (listeners.find(id) == listeners.end())
            {
                listeners[id] = listener;
                listenersCount++;
            }
            else
                VtCoreAssertMsg(false,
                                "Listener with given id already exists!");
        }
        inline void RemoveListener(const char* id)
        {
            listeners.erase(id);
            listenersCount--;
        }
        inline void operator()(Arg1 arg1)
        {
            eventsData.push(arg1);
            EventSystem::PushEvent(this);
        }
        inline void Dispatch() override
        {
            Arg1& arg1 = eventsData.front();
            for (const auto& listener : listeners)
            {
                if (listener.second)
                {
                    if (listener.second(arg1)) break;
                }
            }
            eventsData.pop();
        }

      private:
        int listenersCount = 0;
        std::unordered_map<const char*, std::function<bool(Arg1)>> listeners;
        std::queue<Arg1>                                           eventsData;
    };

    template <typename Arg1, typename Arg2>
    class Event<Arg1, Arg2> : public EventBase
    {
      public:
        inline void RemoveAllListeners()
        {
            listeners.clear();
            listenersCount = 0;
        }
        [[nodiscard]]
        inline int GetListenersCount() const
        {
            return listenersCount;
        }

        inline void AddListener(const char*                            id,
                                const std::function<bool(Arg1, Arg2)>& listener)
        {
            if (listeners.find(id) == listeners.end())
            {
                listeners[id] = listener;
                listenersCount++;
            }
            else
                VtCoreAssertMsg(false,
                                "Listener with given id already exists!");
        }
        inline void RemoveListener(const char* id)
        {
            listeners.erase(id);
            listenersCount--;
        }
        inline void operator()(Arg1 arg1, Arg2 arg2)
        {
            eventsData.push({arg1, arg2});
            EventSystem::PushEvent(this);
        }
        inline void Dispatch() override
        {
            auto& [arg1, arg2] = eventsData.front();
            for (const auto& listener : listeners)
            {
                if (listener.second)
                {
                    if (listener.second(arg1, arg2)) break;
                }
            }
            eventsData.pop();
        }

      private:
        int listenersCount = 0;
        std::unordered_map<const char*, std::function<bool(Arg1, Arg2)>>
            listeners;
        struct EventData
        {
            Arg1 arg1;
            Arg2 arg2;
        };
        std::queue<EventData> eventsData;
    };
    template <typename Arg1, typename Arg2, typename Arg3>
    class Event<Arg1, Arg2, Arg3> : public EventBase
    {
      public:
        inline void RemoveAllListeners()
        {
            listeners.clear();
            listenersCount = 0;
        }
        [[nodiscard]]
        inline int GetListenersCount() const
        {
            return listenersCount;
        }

        inline void
        AddListener(const char*                                  id,
                    const std::function<bool(Arg1, Arg2, Arg3)>& listener)
        {
            if (listeners.find(id) == listeners.end())
            {
                listeners[id] = listener;
                listenersCount++;
            }
            else
                VtCoreAssertMsg(false,
                                "Listener with given id already exists!");
        }
        inline void RemoveListener(const char* id)
        {
            listeners.erase(id);
            listenersCount--;
        }
        inline void operator()(Arg1 arg1, Arg2 arg2, Arg3 arg3)
        {
            eventsData.push({arg1, arg2, arg3});
            EventSystem::PushEvent(this);
        }
        inline void Dispatch() override
        {
            auto& [arg1, arg2, arg3] = eventsData.front();
            for (const auto& listener : listeners)
            {
                if (listener.second)
                {
                    if (listener.second(arg1, arg2, arg3)) break;
                }
            }
            eventsData.pop();
        }

      private:
        int listenersCount = 0;
        std::unordered_map<const char*, std::function<bool(Arg1, Arg2, Arg3)>>
            listeners;
        struct EventData
        {
            Arg1 arg1;
            Arg2 arg2;
            Arg3 arg3;
        };
        std::queue<EventData> eventsData;
    };
} // namespace Vortex
