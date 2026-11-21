/*
 * Created by v1tr10l7 on 18.11.2026.
 * Copyright (c) 2024-2026, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
module;

#include <Prism/Core/Types.hpp>

#include <filesystem>
#include <string>
#include <string_view>

export module Horizon.Platform;

export import Prism.Containers.Vector;
export import Prism.String.String;
export import Prism.String.StringView;
export import Prism.Utility.Optional;
export import Prism.Utility.Path;
export import Prism.Utility.PathView;

export namespace Horizon
{
    // Prism has no directory listing, removal or renaming, so those go
    // through std::filesystem; these convert at that boundary (and at
    // nlohmann::json's, which speaks std::string).
    namespace fs = std::filesystem;

    inline fs::path ToFs(PathView path) { return fs::path(std::string_view(path.Raw(), path.Size())); }
    inline Path     FromFs(const fs::path& path)
    {
        auto text = path.string();
        return Path(StringView(text.data(), text.size()));
    }
    inline std::string ToStd(StringView text) { return std::string(text.Raw(), text.Size()); }
    inline String      FromStd(std::string_view text) { return String(StringView(text.data(), text.size())); }

    inline bool        Exists(PathView path)
    {
        std::error_code ec;
        return fs::exists(ToFs(path), ec);
    }
    inline bool IsDirectory(PathView path)
    {
        std::error_code ec;
        return fs::is_directory(ToFs(path), ec);
    }
    /// The file name without its extension ("Game" for ".../Game.vproj").
    inline StringView Stem(PathView path)
    {
        StringView base = path.BaseName();
        usize      dot  = base.FindLastOf('.');
        return dot == StringView::NPos || dot == 0 ? base : base.Substr(0, dot);
    }
    /// Where Horizon keeps horizon.json ($XDG_CONFIG_HOME/vortex,
    /// %APPDATA%\Vortex).
    Path   ConfigDirectory();
    /// Where engine installs and editor logs go by default
    /// ($XDG_DATA_HOME/vortex, %LOCALAPPDATA%\Vortex).
    Path   DataDirectory();
    /// The user's home directory (Documents-like default for projects).
    Path   HomeDirectory();
    /// The directory holding the running executable.
    Path   ExecutableDirectory();

    /// "linux-x86_64", "windows-x86_64"...; matches manifest artifacts.
    String HostPlatform();
    /// "Nexus" or "Nexus.exe".
    String EditorExecutableName();

    struct LaunchRequest
    {
        Path           Executable;
        Vector<String> Arguments;
        /// Nexus loads assets/ and DotNet/ relative to its working directory.
        Path           WorkingDirectory;
        /// Prepended to LD_LIBRARY_PATH (PATH on Windows); empty = none.
        Path           LibraryDirectory;
        /// stdout and stderr go here; empty = inherited.
        Path           LogFile;
    };

    /// Starts a process that outlives the launcher: it gets its own session
    /// and isn't waited for. Returns an error message, or empty on success.
    String SpawnDetached(const LaunchRequest& request);

    /// Runs `argv` to completion; returns its exit code (-1 if it couldn't
    /// start). Its stdout and stderr are appended to `output` when given.
    int    RunAndWait(const Vector<String>& argv, String* output = nullptr);
} // namespace Horizon
