/*
 * Created by v1tr10l7 on 07.06.2026.
 * Copyright (c) 2024-2026, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include <Prism/Containers/UnorderedMap.hpp>
#include <Prism/Utility/Path.hpp>
#include <Vortex/Renderer/API/Shader.hpp>

namespace Vortex
{
    class ShaderLibrary : public RefCounted
    {
      public:
        ShaderLibrary();
        ~ShaderLibrary();

        inline void          Clear() { m_Shaders.Clear(); }

        void                 Add(::Ref<Shader> shader);
        void                 Add(StringView name, ::Ref<Shader> shader);
        void                 Load(PathView path, bool forceCompile = false,
                                  bool disableOptimization = false);
        void                 Load(StringView name, PathView path);

        const ::Ref<Shader>& Get(StringView name) const;
        usize                Size() const { return m_Shaders.Size(); }

        UnorderedMap<String, ::Ref<Shader>>& ShaderMap() { return m_Shaders; }
        const UnorderedMap<String, ::Ref<Shader>>& ShaderMap() const
        {
            return m_Shaders;
        }

      private:
        UnorderedMap<String, ::Ref<Shader>> m_Shaders;
    };
}; // namespace Vortex
