/*
 * Created by v1tr10l7 on 21.05.2026.
 * Copyright (c) 2024-2026, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

namespace Vortex
{
    using EventID = u64;

    struct EventType 
    {
        static EventID NextID(){static EventID id = 0; return id++;}  
    };

    template <typename T>
    struct EventMetadata 
    {
        static const EventID ID;
    };
    
    template <typename T>
    const EventID EventMetadata<T>::ID = EventType::NextID(); 

    struct EventHeader
    {
        EventID ID;  
        u32 Size;
    };
};
