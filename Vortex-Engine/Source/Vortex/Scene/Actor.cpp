/*
 * Created by v1tr10l7 on 24.10.2026.
 * Copyright (c) 2024-2026, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#include <Vortex/Scene/Components.hpp>
#include <Vortex/Scene/Actor.hpp>

namespace Vortex
{
    StringView Actor::Name()
    {
        if (!HasComponent<TagComponent>()) return "Unnamed Entity";

        return GetComponent<TagComponent>().Name.View();
    }
}; // namespace Vortex
