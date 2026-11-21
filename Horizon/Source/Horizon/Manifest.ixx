/*
 * Created by v1tr10l7 on 18.11.2026.
 * Copyright (c) 2024-2026, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
module;

#include <Prism/Core/Types.hpp>

#include <nlohmann/json.hpp>

export module Horizon.Manifest;

import Prism.Algorithm.Sort;
import Prism.String.Formatter;
import Horizon.Json;
import Horizon.Platform;

/*
 * The release manifest the launcher downloads (see Horizon/Server):
 *
 *   {
 *     "schema": 1,
 *     "releases": [{
 *       "version": "0.2.0", "channel": "stable", "date": "2026-11-18",
 *       "notes": "...",
 *       "components": { "nexus": "0.2.0", "vortex": "0.2.0" },
 *       "artifacts": [{
 *         "platform": "linux-x86_64",
 *         "url": "releases/0.2.0/vortex-0.2.0-linux-x86_64.tar.gz",
 *         "size": 123, "sha256": "...", "executable": "Nexus"
 *       }]
 *     }]
 *   }
 *
 * Relative URLs resolve against the manifest's URL, so the same files work
 * from a local dev server, file://, or real hosting.
 */
export namespace Horizon
{
    /// The newest schema this launcher understands.
    constexpr int kManifestSchema = 1;

    struct Artifact
    {
        String Platform;
        String Url;
        u64    Size = 0;
        String Sha256;
        /// Relative to the unpacked release root.
        String Executable;
    };

    struct Release
    {
        String           Version;
        String           Channel = "stable";
        String           Date;
        String           Notes;
        String           NexusVersion;
        String           VortexVersion;
        Vector<Artifact> Artifacts;

        bool             IsStable() const { return Channel == "stable"; }
        const Artifact*  ForPlatform(StringView platform) const
        {
            for (const auto& artifact : Artifacts)
                if (artifact.Platform.View() == platform) return &artifact;
            return nullptr;
        }
    };

    struct Manifest
    {
        int             Schema = 0;
        /// Newest first.
        Vector<Release> Releases;
    };

    /// Compares dotted versions numerically ("0.10.0" > "0.9.1"); a
    /// pre-release suffix ("0.2.0-nightly") sorts before the plain version.
    int CompareVersions(StringView a, StringView b)
    {
        // The next numeric part of `v`, consuming it and its dot.
        auto nextPart = [](StringView& v) -> u64
        {
            u64   n = 0;
            usize i = 0;
            for (; i < v.Size() && v[i] != '.'; ++i)
                if (v[i] >= '0' && v[i] <= '9') n = n * 10 + static_cast<u64>(v[i] - '0');
            v = i < v.Size() ? v.Substr(i + 1) : StringView();
            return n;
        };
        auto splitSuffix = [](StringView& v) -> StringView
        {
            usize dash = v.Find('-');
            if (dash == StringView::NPos) return {};
            StringView suffix = v.Substr(dash);
            v                 = v.Substr(0, dash);
            return suffix;
        };

        StringView sa = splitSuffix(a), sb = splitSuffix(b);
        while (!a.Empty() || !b.Empty())
        {
            u64 x = nextPart(a), y = nextPart(b);
            if (x != y) return x < y ? -1 : 1;
        }
        if (sa.Empty() != sb.Empty()) return sa.Empty() ? 1 : -1;
        int order = sa.Compare(sb);
        return order < 0 ? -1 : order > 0 ? 1 : 0;
    }

    /// `relative` against the URL of the document that named it.
    String ResolveUrl(StringView base, StringView relative)
    {
        if (relative.Find("://") != StringView::NPos) return String(relative);
        usize scheme = base.Find("://");
        if (relative.StartsWith('/'))
        {
            // Absolute path: keep scheme://host.
            if (scheme == StringView::NPos) return String(relative);
            usize pathStart = base.Find('/', scheme + 3);
            return Format("{}{}", base.Substr(0, pathStart), relative);
        }
        usize slash = base.FindLastOf('/');
        if (slash == StringView::NPos || (scheme != StringView::NPos && slash < scheme + 3))
            return Format("{}/{}", base, relative);
        return Format("{}{}", base.Substr(0, slash + 1), relative);
    }

    Optional<Manifest> ParseManifest(StringView text, StringView manifestUrl, String& error)
    {
        Json::Value root = Json::Parse(text);
        if (root.is_discarded() || !root.is_object())
        {
            error = "The manifest isn't valid JSON";
            return NullOpt;
        }

        Manifest manifest;
        manifest.Schema = Json::Get(root, "schema", 0);
        if (manifest.Schema < 1 || manifest.Schema > kManifestSchema)
        {
            error = Format("Unsupported manifest schema {} (this launcher reads up to {}); "
                           "update Horizon",
                           manifest.Schema, kManifestSchema);
            return NullOpt;
        }

        for (const auto& entry : Json::GetArray(root, "releases"))
        {
            if (!entry.is_object() || !entry.contains("version")) continue;
            Release release;
            release.Version       = Json::GetString(entry, "version");
            release.Channel       = Json::GetString(entry, "channel", "stable");
            release.Date          = Json::GetString(entry, "date");
            release.Notes         = Json::GetString(entry, "notes");
            const auto& parts     = Json::GetObject(entry, "components");
            release.NexusVersion  = Json::GetString(parts, "nexus", release.Version);
            release.VortexVersion = Json::GetString(parts, "vortex", release.Version);

            for (const auto& a : Json::GetArray(entry, "artifacts"))
            {
                if (!a.is_object() || !a.contains("url")) continue;
                Artifact artifact;
                artifact.Platform   = Json::GetString(a, "platform");
                artifact.Url        = ResolveUrl(manifestUrl, Json::GetString(a, "url"));
                artifact.Size       = Json::Get(a, "size", u64{0});
                artifact.Sha256     = Json::GetString(a, "sha256");
                artifact.Executable = Json::GetString(a, "executable");
                release.Artifacts.PushBack(Move(artifact));
            }
            manifest.Releases.PushBack(Move(release));
        }

        Prism::Sort(manifest.Releases.begin(), manifest.Releases.end(),
             [](const Release& a, const Release& b)
             { return CompareVersions(a.Version, b.Version) > 0; });
        return manifest;
    }
} // namespace Horizon
