/*
 * Created by v1tr10l7 on 10.06.2026.
 * Copyright (c) 2024-2026, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include <Prism/Core/Error.hpp>
#include <Prism/Utility/Path.hpp>

namespace Vortex
{
    namespace Filesystem
    {
        enum class AccessMode
        {
            eRead    = Bit(0),
            eWrite   = Bit(1),
            eExecute = Bit(2),
        };

        Path          AbsolutePath(PathView path);

        bool          Exists(PathView path);
        bool          HasAccess(PathView path, AccessMode mode);
        ErrorOr<void> CreateDirectories(PathView path);
    }; // namespace Filesystem
}; // namespace Vortex
