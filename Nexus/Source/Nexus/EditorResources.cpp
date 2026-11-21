/*
 * Created by v1tr10l7 on 15.09.2026.
 * Copyright (c) 2024-2026, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#include <Nexus/EditorResources.hpp>
import Prism.Containers.UnorderedMap;

namespace Vortex::Editor::Resources
{
    static UnorderedMap<String, ::Ref<TextureAsset>> s_TextureCache;

    ::Ref<TextureAsset>                              LoadTexture(PathView path)
    {
        auto it = s_TextureCache.Find(path.StrView());
        if (it != s_TextureCache.end()) return it->Value;

        auto texture                   = CreateRef<TextureAsset>(path);
        s_TextureCache[path.StrView()] = texture;

        return texture;
    }

    void Clear() { s_TextureCache.Clear(); }
}; // namespace Vortex::Editor::Resources
