/*
 * Created by v1tr10l7 on 20.08.2024.
 * Copyright (c) 2024-2024, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include <Prism/Utility/Path.hpp>
#include <Vortex/Core/Core.hpp>
#include <Vortex/Renderer/Image.hpp>

namespace Vortex
{
    class VT_API Texture2D : public NonCopyable<Texture2D>, public RefCounted
    {
      public:
        Texture2D(PathView path) PM_NOEXCEPT : m_Path(path) {}
        virtual ~Texture2D() = default;

        inline PathView         FilePath() const PM_NOEXCEPT { return m_Path; }

        virtual u32             MipLevelCount() = 0;

        static ::Ref<Texture2D> Create(PathView path);

      protected:
        Path m_Path;
    };
}; // namespace Vortex
