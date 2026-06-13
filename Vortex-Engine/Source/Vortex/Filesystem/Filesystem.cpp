/*
 * Created by v1tr10l7 on 10.06.2026.
 * Copyright (c) 2024-2026, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#include <Vortex/Filesystem/Filesystem.hpp>

#include <filesystem>

namespace Vortex
{
    namespace Filesystem
    {
        Path AbsolutePath(PathView path)
        {
            auto absolutePath = std::filesystem::absolute(path.Raw());

            return absolutePath.c_str();
        }

        bool Exists(PathView path)
        {
            return std::filesystem::exists(path.Raw());
        }
        bool HasAccess(PathView path, AccessMode mode)
        {
            IgnoreUnused(path);
            IgnoreUnused(mode);
            return true;
        }
        ErrorOr<void> CreateDirectories(PathView path)
        {
            std::error_code errorCode;
            std::filesystem::create_directories(path.Raw(), errorCode);
            if (errorCode) return Error(errorCode.value());

            return {};
        }
    }; // namespace Filesystem
}; // namespace Vortex
