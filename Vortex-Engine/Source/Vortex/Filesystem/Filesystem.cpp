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
        static UnorderedMap<String, Path>                   s_MountPoints;

        const UnorderedMap<String, Path>& VT_FILESYSTEM_API MountPoints()
        {
            return s_MountPoints;
        }

        bool VT_FILESYSTEM_API Mount(PathView virtualPath,
                                     PathView physicalPath)
        {
            String str;
            if (s_MountPoints.Contains(virtualPath.Raw())) return false;
            s_MountPoints[virtualPath.Raw()] = physicalPath;

            return true;
        }
        bool VT_FILESYSTEM_API Unmount(PathView virtualPath)
        {
            if (!s_MountPoints.Contains(virtualPath.Raw())) return false;
            s_MountPoints.Erase(virtualPath.Raw());

            return true;
        }
        void VT_FILESYSTEM_API UnmountAll() { s_MountPoints.Clear(); }

        Path VT_FILESYSTEM_API Resolve(PathView path)
        {
            String rawPath         = path.Raw();
            String bestMatchPrefix = "";
            Path   bestMatchPhysicalPath;

            // Find the longest matching mount point prefix
            for (const auto& [virtualPrefix, physicalPath] : s_MountPoints)
            {
                if (rawPath.StartsWith(virtualPrefix)
                    && virtualPrefix.Size() > bestMatchPrefix.Size())
                {
                    bestMatchPrefix       = virtualPrefix;
                    bestMatchPhysicalPath = physicalPath;
                }
            }

            if (bestMatchPrefix.Empty()) return Path(rawPath);

            // Strip the matched virtual prefix and append the remaining
            // relative path
            String relativePart = rawPath.Substr(bestMatchPrefix.Size());

            if (!relativePart.Empty()
                && (relativePart[0] == '/' || relativePart[0] == '\\'))
                relativePart = relativePart.Substr(1);

            Path resolvedPath
                = (std::filesystem::path(bestMatchPhysicalPath.Raw())
                   / relativePart.Raw())
                      .string()
                      .data();
            return resolvedPath;
        }

        Path VT_FILESYSTEM_API AbsolutePath(PathView path)
        {
            auto absolutePath = std::filesystem::absolute(path.Raw());

            return absolutePath.c_str();
        }

        bool VT_FILESYSTEM_API Exists(PathView path)
        {
            return std::filesystem::exists(path.Raw());
        }
        bool VT_FILESYSTEM_API IsDirectory(PathView path)
        {
            return std::filesystem::is_directory(path.Raw());
        }
        bool VT_FILESYSTEM_API IsEmpty(PathView path)
        {
            return std::filesystem::is_empty(path.Raw());
        }

        bool VT_FILESYSTEM_API HasAccess(PathView path, AccessMode mode)
        {
            IgnoreUnused(path);
            IgnoreUnused(mode);

            return true;
        }
        ErrorOr<void> VT_FILESYSTEM_API CreateDirectories(PathView path)
        {
            std::error_code errorCode;
            std::filesystem::create_directories(path.Raw(), errorCode);
            if (errorCode) return Error(errorCode.value());

            return {};
        }
    }; // namespace Filesystem
}; // namespace Vortex
