/*
 * Created by v1tr10l7 on 21.09.2024.
 * Copyright (c) 2024-2024, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include <Vortex/Renderer/API/Shader.hpp>

namespace Vortex
{
    class VT_API Material : public NonCopyable<Material>, public RefCounted
    {
      public:
        Material()          = default;
        virtual ~Material() = default;

        virtual void Set(const String& name, ::Ref<Texture2D> texture)    = 0;
        virtual void Set(const String& name, ::Ref<UniformBuffer> buffer) = 0;

        static ::Ref<Material> Create(const ::Ref<Shader> shader);
    };
}; // namespace Vortex
