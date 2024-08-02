/*
 * Created by v1tr10l7 on 02.08.2024.
 * Copyright (c) 2024-2024, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include "Vortex/Core/Core.hpp"

namespace Vortex
{
    using Pixel = u8;

    namespace ImageLoader
    {
        VT_API std::expected<Scope<Pixel[]>, std::string>
               LoadBMP(std::filesystem::path path, i32& width, i32& height);
    }
}; // namespace Vortex
