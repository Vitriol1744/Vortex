/*
 * Created by v1tr10l7 on 02.08.2024.
 * Copyright (c) 2024-2024, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include <Prism/Memory/Scope.hpp>
#include <Prism/Utility/Expected.hpp>
#include <Prism/Utility/Path.hpp>
#include <Vortex/Core/Core.hpp>

namespace Vortex
{
    using Pixel = u8;

    namespace ImageLoader
    {
        VT_API Expected<Scope<Pixel>, String> LoadBMP(Path path, i32& width,
                                                      i32& height);
    }
}; // namespace Vortex
