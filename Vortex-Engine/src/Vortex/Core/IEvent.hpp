//
// Created by vitriol1744 on 08.07.2021.
//
#pragma once

#include "Core/Core.hpp"
#include "Core/EventSystem.hpp"

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

            inline void AddListener(const char* id, const std::function<void()>& listener)
            {
                if (listeners.find(id) == listeners.end())
                {
                    listeners[id] = listener;
                    listenersCount++;
                }
                else VT_CORE_ASSERT_MSG(false, "Listener with given id already exists!");
            }
            inline void RemoveListener(const char* id, const std::function<void()>& listener)
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
            std::unordered_map<const char*, std::function<void()>> listeners;
    };

    template<typename T>
    class Event<T> : public IEventBase
    {
        public:
            inline void RemoveAllListeners() { listeners.clear(); listenersCount = 0;}
            VT_NODISCARD inline int GetListenersCount() const { return listenersCount; }

            inline void AddListener(const char* id, const std::function<void(T)>& listener)
            {
                if (listeners.find(id) == listeners.end())
                {
                    listeners[id] = listener;
                    listenersCount++;
                }
                else VT_CORE_ASSERT_MSG(false, "Listener with given id already exists!");
            }
            inline void RemoveListener(const char* id, const std::function<void(T)>& listener)
            {
                listeners.erase(id);
                listenersCount--;
            }
            inline void operator()(T arg1) { this->arg1 = arg1; EventSystem::Instance()->PushEvent(this); }
            inline void Dispatch() override { for (const auto& listener : listeners) if (listener.second) listener.second.operator()(arg1); }        private:
            int listenersCount = 0;
            std::unordered_map<const char*, std::function<void(T)>> listeners;

            T arg1;
    };

    template<typename T1, typename T2>
    class Event<T1, T2> : public IEventBase
    {
        public:
            inline void RemoveAllListeners() { listeners.clear(); listenersCount = 0;}
            VT_NODISCARD inline int GetListenersCount() const { return listenersCount; }

            inline void AddListener(const char* id, const std::function<void(T1, T2)>& listener)
            {
                if (listeners.find(id) == listeners.end())
                {
                    listeners[id] = listener;
                    listenersCount++;
                }
                else VT_CORE_ASSERT_MSG(false, "Listener with given id already exists!");
            }
            inline void RemoveListener(const char* id, const std::function<void(T1, T2)>& listener)
            {
                listeners.erase(id);
                listenersCount--;
            }
            inline void operator()(T1 arg1, T2 arg2) { this->arg1 = arg1; this->arg2 = arg2; EventSystem::Instance()->PushEvent(this); }
            inline void Dispatch() override { for (const auto& listener : listeners) if (listener.second) listener.second.operator()(arg1, arg2); }        private:

            int listenersCount = 0;
            std::unordered_map<const char*, std::function<void(T1, T2)>> listeners;

            T1 arg1;
            T2 arg2;
    };
}