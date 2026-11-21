/*
 * Created by v1tr10l7 on 13.07.2026.
 * Copyright (c) 2024-2026, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include <Vortex/Core/Types.hpp>
#include <Vortex/Renderer/Export.hpp>

import Prism.Containers.UnorderedMap;
import Prism.Containers.Vector;
import Prism.Memory.Ref;
import Prism.String.String;
import Prism.String.StringView;
import Prism.Utility.Path;
import Prism.Utility.PathView;
import Vortex.RHI;

namespace Vortex
{
    /**
     * Named shaders, shared by the renderers and materials.
     *
     * Shaders loaded from a description can be recompiled at runtime
     * (Reload): the name then refers to the new shader, the pipeline cache
     * forgets the old one's pipelines, and Generation() changes so holders
     * of the old shader (materials, renderers) can pick the new one up.
     * Renderers that look their shaders up by name every frame and get their
     * pipelines from Renderer::Pipelines() follow reloads automatically.
     *
     * Game thread only.
     */
    class VT_RENDERER_API ShaderLibrary
    {
      public:
        ShaderLibrary()                                = default;
        ShaderLibrary(const ShaderLibrary&)            = delete;
        ShaderLibrary& operator=(const ShaderLibrary&) = delete;

        void           Clear() { m_Shaders.Clear(); }
        /// Adds (or replaces) `name`. Without a source it can't be reloaded.
        void           Add(StringView name, ::Ref<RHI::Shader> shader);
        /// Compiles `path` and adds it as `name`. Logs and returns false if
        /// it doesn't compile.
        bool           Load(StringView name, PathView path);
        /// Compiles `desc` and adds it as `name`; Reload compiles `desc`
        /// again. Logs and returns false if it doesn't compile.
        bool           Load(StringView name, const RHI::ShaderDesc& desc);

        /// Recompiles `name` from its source. On failure, logs and keeps the
        /// current shader.
        bool           Reload(StringView name);
        /// Reloads every shader that has a source; false if any failed.
        bool           ReloadAll();
        /// Changes whenever a name starts referring to another shader.
        inline u64     Generation() const { return m_Generation; }

        bool           Contains(StringView name) const;
        /// Asserts if there is no such shader.
        ::Ref<RHI::Shader> Get(StringView name) const;
        /// Null if there is no such shader.
        ::Ref<RHI::Shader> Find(StringView name) const;
        usize          Size() const { return m_Shaders.Size(); }
        /// Every name, sorted.
        Vector<String> Names() const;
        /// The name `shader` is registered under; empty if it isn't.
        String         NameOf(const RHI::Shader& shader) const;
        /// The file `name` compiles from; empty for shaders built from
        /// in-memory sources or added directly.
        Path           SourceFile(StringView name) const;
        /// Whether Reload can recompile `name`.
        bool           IsReloadable(StringView name) const;

      private:
        struct Entry
        {
            ::Ref<RHI::Shader> Shader;
            /// What Reload compiles; no file and no sources: not reloadable.
            RHI::ShaderDesc    Source;
            bool               Reloadable = false;
        };

        UnorderedMap<String, Entry> m_Shaders;
        u64                         m_Generation = 0;

        void Replace(Entry& entry, ::Ref<RHI::Shader> shader);
    };
} // namespace Vortex
