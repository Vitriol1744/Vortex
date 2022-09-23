//
// Created by Vitriol1744 on 08.09.2022.
//
#ifndef VORTEX_IEVENT_HPP
#define VORTEX_IEVENT_HPP

#include "Vortex/Core/HashedString.hpp"
#include "Vortex/Core/Export.hpp"
#include "Vortex/Core/Types.hpp"

#define VT_REGISTER_EVENT(name) struct name : public Vortex::IEvent
#define VT_EVENT_CLASS_ID(name) \
        VT_NODISCARD inline static constexpr const  EventID GetStaticEventID() noexcept { return VT_HASH(#name); }                    \
        VT_NODISCARD virtual inline constexpr const EventID GetEventID() const noexcept override { return GetStaticEventID(); } \
        inline static constexpr const char* GetEventName() noexcept { return #name; };
namespace Vortex
{
    using EventID = uint64;
    struct IEvent
    {
        VT_NODISCARD virtual inline constexpr const EventID GetEventID() const noexcept = 0;
    };

    template <typename T, typename F>
    bool DispatchEvent(IEvent& event, const F& func) requires std::derived_from<T, IEvent>
    {
        if (event.GetEventID() == T::GetStaticEventID())
        {
            func(static_cast<T&>(event));
            return true;
        }

        return false;
    }
}

#endif //VORTEX_IEVENT_HPP
