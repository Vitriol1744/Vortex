/*
 * Created by v1tr10l7 on 18.07.2024.
 * Copyright (c) 2024-2024, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include "Vortex/Core/Export.hpp"

namespace Vortex
{
    template <typename T>
    class VT_API NonCopyable
    {
      protected:
        NonCopyable()          = default;
        virtual ~NonCopyable() = default;

      private:
        NonCopyable(const NonCopyable&) = delete;
        T& operator=(const T&)          = delete;
    };
}; // namespace Vortex
