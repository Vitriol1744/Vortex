/*
 * Created by v1tr10l7 on 15.09.2026.
 * Copyright (c) 2024-2026, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include <Vortex/Renderer/TextureAsset.hpp>

namespace Vortex::Editor::Resources
{
    ::Ref<TextureAsset> LoadTexture(PathView path);
};
