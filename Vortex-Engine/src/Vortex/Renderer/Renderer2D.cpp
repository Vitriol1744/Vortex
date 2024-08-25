/*
 * Created by v1tr10l7 on 24.08.2024.
 * Copyright (c) 2024-2024, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#include "vtpch.hpp"

#include "Vortex/Core/Math/Math.hpp"

#include "Vortex/Renderer/API/IndexBuffer.hpp"
#include "Vortex/Renderer/API/Texture2D.hpp"
#include "Vortex/Renderer/API/VertexBuffer.hpp"
#include "Vortex/Renderer/Renderer2D.hpp"

namespace Vortex::Renderer
{
    namespace
    {
        struct Vertex
        {
            Vec3 Position;
            Vec3 Color;
            Vec2 TexCoords;
        };

        inline constexpr u32 MAX_QUADS      = 20000;
        inline constexpr u32 MAX_VERTICES   = MAX_QUADS * 4;
        inline constexpr u32 MAX_INDICES    = MAX_QUADS * 6;

        Ref<VertexBuffer>    s_VertexBuffer = nullptr;
        Ref<IndexBuffer>     s_IndexBuffer  = nullptr;
        Ref<Texture2D>       s_Texture      = nullptr;
    }; // namespace

    void Initialize()
    {
        VT_UNUSED(MAX_VERTICES);
        VT_UNUSED(MAX_INDICES);
    }
    void Shutdown() {}
}; // namespace Vortex::Renderer
