/*
 * Created by v1tr10l7 on 06.09.2024.
 * Copyright (c) 2024-2024, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#ifndef VT_DIST
    #include <tracy/Tracy.hpp>

    #define VtProfileFunction(...) ZoneScoped##__VA_OPT__(N(__VA_ARGS__))
    #define VtProfileScope(...)    VtProfileFunction(__VA_ARGS__)
    #define VtProfileScopeDynamic(NAME)                                        \
        ZoneScoped;                                                            \
        ZoneName(NAME, strlen(NAME))
    #define VtProfilerSetThreadName(name) tracy::SetThreadName(name);
    /// End of a frame (Tracy's frame timeline).
    #define VtProfileFrameMark            FrameMark
    /// End of a frame of a secondary timeline (e.g. the render thread's).
    #define VtProfileFrameMarkNamed(name) FrameMarkNamed(name)
#else
    #define VtProfileFunction(...)
    #define VtProfileScope(...)
    #define VtProfileScopeDynamic(NAME)
    #define VtProfilerSetThreadName(name)
    #define VtProfileFrameMark
    #define VtProfileFrameMarkNamed(name)
#endif
