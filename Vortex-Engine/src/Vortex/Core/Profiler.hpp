/*
 * Created by v1tr10l7 on 06.09.2024.
 * Copyright (c) 2024-2024, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#ifndef VT_DIST
    #include <tracy/Tracy.hpp>

    #define VtProfileFunction(...)        ZoneScoped##__VA_OPT__(N(__VA_ARGS__))
    #define VtProfileScope(...)           VtProfileFunction(__VA_ARGS__)
    #define VtProfilerSetThreadName(name) tracy::SetThreadName(name);
#else
    #define VtProfileFunction(...)
    #define VtProfileScope(...)
    #define VtProfilerSetThreadName(name)
#endif
