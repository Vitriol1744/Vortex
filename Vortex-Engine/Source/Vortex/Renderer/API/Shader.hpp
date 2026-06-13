/*
 * Created by v1tr10l7 on 28.07.2024.
 * Copyright (c) 2024-2024, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include <Vortex/Core/Core.hpp>
#include <Prism/Utility/PathView.hpp>

namespace Vortex
{
    class VT_API Shader : public NonCopyable<Shader>, public RefCounted
    {
      public:
        Shader()          = default;
        virtual ~Shader() = default;

        inline StringView Name() const { return m_Name; }

        virtual void      SetUniform(const String&              name,
                                     ::Ref<class UniformBuffer> buffer)
            = 0;
        virtual void SetUniform(const String&          name,
                                ::Ref<class Texture2D> texture)
            = 0;

        static ::Ref<Shader> Create(PathView path);
        static ::Ref<Shader> Create(PathView vertexPath, PathView fragmentPath);

      protected:
        String m_Name;
    };
}; // namespace Vortex
