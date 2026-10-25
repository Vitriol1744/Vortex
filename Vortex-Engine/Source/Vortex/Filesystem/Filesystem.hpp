/*
 * Created by v1tr10l7 on 10.06.2026.
 * Copyright (c) 2024-2026, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include <Vortex/Filesystem/Export.hpp>

#include <Prism/Containers/UnorderedMap.hpp>
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

        const VT_FILESYSTEM_API UnorderedMap<String, Path>& MountPoints();

        bool VT_FILESYSTEM_API Mount(PathView virtualPath,
                                     PathView physicalPath);
        bool VT_FILESYSTEM_API Unmount(PathView virtualPath);
        void VT_FILESYSTEM_API UnmountAll();

        bool VT_FILESYSTEM_API Unmount(PathView virtualPath);
        Path VT_FILESYSTEM_API Resolve(PathView path);

        Path VT_FILESYSTEM_API AbsolutePath(PathView path);

        bool VT_FILESYSTEM_API Exists(PathView path);
        bool VT_FILESYSTEM_API IsDirectory(PathView path);
        bool VT_FILESYSTEM_API IsEmpty(PathView path);

        bool VT_FILESYSTEM_API HasAccess(PathView path, AccessMode mode);
        ErrorOr<void> VT_FILESYSTEM_API CreateDirectories(PathView path);
    }; // namespace Filesystem
}; // namespace Vortex
