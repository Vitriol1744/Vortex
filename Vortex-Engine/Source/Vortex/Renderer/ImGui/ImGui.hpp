/*
 * Created by v1tr10l7 on 30.05.2026.
 * Copyright (c) 2024-2026, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include <Vortex/Renderer/API/SwapChain.hpp>
#include <Vortex/Renderer/API/Texture2D.hpp>

namespace Vortex
{
    namespace UI
    {
        void*         GetTexture(Ref<Texture2D> texture);
        Vector<void*> GetTextures(Ref<SwapChain> swapChain);
    }; // namespace UI
}; // namespace Vortex
