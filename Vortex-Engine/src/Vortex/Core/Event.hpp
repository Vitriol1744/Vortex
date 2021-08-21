//
// Created by vitriol1744 on 08.07.2021.
//
#pragma once

#define VTAddListener2(event, name, function, ...) event.AddListener(std::to_string(name).data(), std::bind(function, __VA_ARGS__))
#define VTAddListener(event, function, ...) VTAddListener2(event, __COUNTER__, function, __VA_ARGS__)
#define VT_PH(n) std::placeholders::_##n

#include "Vortex/Core/Core.hpp"
#include "Vortex/Core/EventSystem.hpp"

#include <vector>
#include <unordered_map>
#include <functional>

namespace Vortex
{
    class IEventBase
    {
        public:
            virtual ~IEventBase() = default;

            virtual void Dispatch() = 0;
    };

    template<typename... Args>
    class Event : public IEventBase
    {
        public:
            inline void RemoveAllListeners() { listeners.clear(); listenersCount = 0;}
            VT_NODISCARD inline int GetListenersCount() const { return listenersCount; }

            inline void AddListener(const char* id, const std::function<bool()>& listener)
            {
                if (listeners.find(id) == listeners.end())
                {
                    listeners[id] = listener;
                    listenersCount++;
                }
                else VT_CORE_ASSERT_MSG(false, "Listener with given id already exists!");
            }
            inline void RemoveListener(const char* id, const std::function<bool()>& listener)
            {
                listeners.erase(id);
                listenersCount--;
            }
            inline void operator()()
            {
                EventSystem::Instance()->PushEvent(this);
            }
            inline void Dispatch() override { for (const auto& listener : listeners) if (listener.second) listener.second.operator()(); }

        private:
            int listenersCount = 0;
            std::unordered_map<const char*, std::function<bool()>> listeners;
    };

    template<typename Arg1>
    class Event<Arg1> : public IEventBase
    {
        public:
            inline void RemoveAllListeners() { listeners.clear(); listenersCount = 0;}
            VT_NODISCARD inline int GetListenersCount() const { return listenersCount; }

            inline void AddListener(const char* id, const std::function<bool(Arg1)>& listener)
            {
                if (listeners.find(id) == listeners.end())
                {
                    listeners[id] = listener;
                    listenersCount++;
                }
                else VT_CORE_ASSERT_MSG(false, "Listener with given id already exists!");
            }
            inline void RemoveListener(const char* id, const std::function<bool(Arg1)>& listener)
            {
                listeners.erase(id);
                listenersCount--;
            }
            inline void operator()(Arg1 arg1) { eventsData.push(arg1); EventSystem::Instance()->PushEvent(this); }
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
            std::queue<Arg1> eventsData;
    };

    template<typename Arg1, typename Arg2>
    class Event<Arg1, Arg2> : public IEventBase
    {
        public:
            inline void RemoveAllListeners() { listeners.clear(); listenersCount = 0;}
            VT_NODISCARD inline int GetListenersCount() const { return listenersCount; }

            inline void AddListener(const char* id, const std::function<bool(Arg1, Arg2)>& listener)
            {
                if (listeners.find(id) == listeners.end())
                {
                    listeners[id] = listener;
                    listenersCount++;
                }
                else VT_CORE_ASSERT_MSG(false, "Listener with given id already exists!");
            }
            inline void RemoveListener(const char* id, const std::function<bool(Arg1, Arg2)>& listener)
            {
                listeners.erase(id);
                listenersCount--;
            }
            inline void operator()(Arg1 arg1, Arg2 arg2)
            {
                eventsData.push({arg1, arg2});
                EventSystem::Instance()->PushEvent(this);
            }
            inline void Dispatch() override
            {
                auto&[arg1, arg2] = eventsData.front();
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
            std::unordered_map<const char*, std::function<bool(Arg1, Arg2)>> listeners;
            struct EventData
            {
                Arg1 arg1;
                Arg2 arg2;
            };
            std::queue<EventData> eventsData;
    };
    template<typename Arg1, typename Arg2, typename Arg3>
    class Event<Arg1, Arg2, Arg3> : public IEventBase
    {
        public:
            inline void RemoveAllListeners() { listeners.clear(); listenersCount = 0;}
            VT_NODISCARD inline int GetListenersCount() const { return listenersCount; }

            inline void AddListener(const char* id, const std::function<bool(Arg1, Arg2, Arg3)>& listener)
            {
                if (listeners.find(id) == listeners.end())
                {
                    listeners[id] = listener;
                    listenersCount++;
                }
                else VT_CORE_ASSERT_MSG(false, "Listener with given id already exists!");
            }
            inline void RemoveListener(const char* id, const std::function<bool(Arg1, Arg2, Arg3)>& listener)
            {
                listeners.erase(id);
                listenersCount--;
            }
            inline void operator()(Arg1 arg1, Arg2 arg2, Arg3 arg3)
            {
                eventsData.push({arg1, arg2, arg3});
                EventSystem::Instance()->PushEvent(this);
            }
            inline void Dispatch() override
            {
                auto&[arg1, arg2, arg3] = eventsData.front();
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
            std::unordered_map<const char*, std::function<bool(Arg1, Arg2, Arg3)>> listeners;
            struct EventData
            {
                Arg1 arg1;
                Arg2 arg2;
                Arg3 arg3;
            };
            std::queue<EventData> eventsData;
    };
}