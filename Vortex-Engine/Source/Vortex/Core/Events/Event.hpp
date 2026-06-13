/*
 * Created by v1tr10l7 on 18.07.2024.
 * Copyright (c) 2024-2024, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include <Prism/Algorithm/Find.hpp>
#include <Prism/Containers/Queue.hpp>
#include <Prism/Containers/Vector.hpp>

#include <Vortex/Core/Assertions.hpp>
#include <Vortex/Core/Events/EventSystem.hpp>

namespace Vortex
{
    using EventID = u32;
    struct EventType
    {
        static EventID NextID()
        {
            static EventID id = 0;

            return ++id;
        }
    };

    enum class EventCategory : u8
    {
        eSystem,
        eInput,
        eUser,
        eRender,
    };
    template <typename T>
    concept IsEvent = requires {
        { T::Category } -> SameAs<const EventCategory&>;
    };

    template <typename T>
    struct EventMetadata
    {
        static const EventID           ID;
        static constexpr EventCategory Category = T::Category;
    };
    template <typename T>
    const EventID EventMetadata<T>::ID = EventType::NextID();

    struct EventHeader
    {
        EventID ID;
        u32     Size;
    };

    template <typename T, typename... U>
    usize GetFunctionAddress(std::function<T(U...)> f)
    {
        typedef T(fnType)(U...);
        fnType** fnPointer = f.template target<fnType*>();
        return (usize)*fnPointer;
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

        inline void RemoveAllListeners() { m_Listeners.Clear(); }
        PM_NODISCARD
        inline int  GetListenerCount() const { return m_Listeners.Size(); }

        inline void AddListener(EventListener listener)
        {
            m_Listeners.PushBack(listener);
        }
        inline void operator+=(EventListener listener)
        {
            AddListener(listener);
        }
        inline void operator-=(EventListener listener)
        {
            (void)RemoveIf(m_Listeners.begin(), m_Listeners.end(),
                           [&listener](const EventListener& currentListener)
                           {
                               return listener.target<bool()>()
                                   == currentListener.target<bool()>();
                           });
        }
        inline void operator()() { Dispatch(); }
        inline void Dispatch() override
        {
            for (const auto& listener : m_Listeners)
                if (listener && listener()) break;
        }

      private:
        Vector<EventListener> m_Listeners;
    };

    template <typename Arg1>
    class Event<Arg1> : public EventBase
    {
      public:
        using EventListener = std::function<bool(Arg1)>;

        inline void RemoveAllListeners() { m_Listeners.Clear(); }
        PM_NODISCARD
        inline int  GetListenerCount() const { return m_Listeners.Size(); }

        inline void AddListener(EventListener listener)
        {
            m_Listeners.PushBack(listener);
        }
        inline void operator+=(EventListener listener)
        {
            AddListener(listener);
        }
        inline void operator-=(EventListener listener)
        {
            (void)RemoveIf(m_Listeners.begin(), m_Listeners.end(),
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
        Vector<std::function<bool(Arg1)>> m_Listeners;
        PM_UNUSED
        Queue<Arg1> m_EventData;
    };

    template <typename Arg1, typename Arg2>
    class Event<Arg1, Arg2> : public EventBase
    {
      public:
        using EventListener = std::function<bool(Arg1, Arg2)>;

        inline void RemoveAllListeners() { m_Listeners.Clear(); }
        PM_NODISCARD
        inline int  GetListenerCount() const { return m_Listeners; }

        inline void AddListener(const EventListener& listener)
        {
            m_Listeners.PushBack(listener);
        }
        inline void operator+=(const EventListener& listener)
        {
            AddListener(listener);
        }
        inline void operator-=(EventListener listener)
        {
            (void)RemoveIf(m_Listeners.begin(), m_Listeners.end(),
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
        Vector<EventListener> m_Listeners;
        struct EventData
        {
            Arg1 arg1;
            Arg2 arg2;
        };
        PM_UNUSED
        Queue<EventData> m_EventData;
    };
    template <typename Arg1, typename Arg2, typename Arg3>
    class Event<Arg1, Arg2, Arg3> : public EventBase
    {
      public:
        using EventListener = std::function<bool(Arg1, Arg2, Arg3)>;

        inline void RemoveAllListeners() { m_Listeners.Clear(); }
        PM_NODISCARD
        inline int  GetListenerCount() const { return m_Listeners.Size(); }

        inline void AddListener(const EventListener& listener)
        {
            m_Listeners.PushBack(listener);
        }
        inline void operator+=(EventListener listener)
        {
            m_Listeners.PushBack(listener);
        }
        inline void operator-=(EventListener listener)
        {
            (void)RemoveIf(m_Listeners.begin(), m_Listeners.end(),
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
        Vector<EventListener> m_Listeners;
        struct EventData
        {
            Arg1 arg1;
            Arg2 arg2;
            Arg3 arg3;
        };
        PM_UNUSED
        Queue<EventData> m_EventData;
    };
} // namespace Vortex
