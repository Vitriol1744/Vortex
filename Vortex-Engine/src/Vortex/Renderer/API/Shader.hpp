/*
 * Created by v1tr10l7 on 28.07.2024.
 * Copyright (c) 2024-2024, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include "Vortex/Core/Core.hpp"

namespace Vortex
{
    class VT_API Shader
    {
      public:
        Shader()          = default;
        virtual ~Shader() = default;

        virtual void SetUniform(const std::string&       name,
                                Ref<class UniformBuffer> buffer)
            = 0;
        virtual void SetUniform(const std::string&   name,
                                Ref<class Texture2D> texture)
            = 0;

        static Ref<Shader> Create(std::string_view path);
        static Ref<Shader> Create(std::string_view vertexPath,
                                  std::string_view fragmentPath);
    };
}; // namespace Vortex
