/*
 * Created by v1tr10l7 on 13.07.2026.
 * Copyright (c) 2024-2026, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#include <Vortex/Core/Assertions.hpp>
#include <Vortex/Renderer/ShaderLibrary.hpp>

import Prism.Algorithm.Sort;
import Prism.Containers.Vector;
import Prism.Utility.Path;
import Vortex.Renderer;

namespace Vortex
{
    void ShaderLibrary::Replace(Entry& entry, ::Ref<RHI::Shader> shader)
    {
        if (entry.Shader == shader) return;
        // Pipelines of the old shader are of no use anymore; draws already
        // recorded keep theirs (and the shader) alive until the GPU is done.
        if (entry.Shader) Renderer::Pipelines().Invalidate(*entry.Shader);
        entry.Shader = Move(shader);
        ++m_Generation;
    }

    void ShaderLibrary::Add(StringView name, ::Ref<RHI::Shader> shader)
    {
        VtCoreAssertMsg(shader, "ShaderLibrary: Adding a null shader");
        String key(name);
        if (m_Shaders.Contains(key))
            VtCoreWarn("ShaderLibrary: Replacing shader '{}'", name);
        Entry& entry     = m_Shaders[key];
        entry.Reloadable = false;
        entry.Source     = {};
        Replace(entry, Move(shader));
    }

    bool ShaderLibrary::Load(StringView name, PathView path)
    {
        RHI::ShaderDesc desc{};
        desc.File = Path(path);
        return Load(name, desc);
    }

    bool ShaderLibrary::Load(StringView name, const RHI::ShaderDesc& source)
    {
        RHI::ShaderDesc desc = source;
        if (desc.DebugName.Empty()) desc.DebugName = String(name);
        auto shader = Renderer::Device().CreateShader(desc);
        if (!shader)
        {
            VtCoreError("ShaderLibrary: {}", shader.Error());
            return false;
        }

        String key(name);
        if (m_Shaders.Contains(key))
            VtCoreWarn("ShaderLibrary: Replacing shader '{}'", name);
        Entry& entry     = m_Shaders[key];
        entry.Source     = Move(desc);
        entry.Reloadable = true;
        Replace(entry, *shader);
        return true;
    }

    bool ShaderLibrary::Reload(StringView name)
    {
        auto it = m_Shaders.Find(String(name));
        if (it == m_Shaders.end())
        {
            VtCoreWarn("ShaderLibrary: No shader named '{}' to reload", name);
            return false;
        }
        Entry& entry = it->Value;
        if (!entry.Reloadable)
        {
            VtCoreWarn("ShaderLibrary: '{}' wasn't loaded from a source; can't "
                       "reload it",
                       name);
            return false;
        }

        auto shader = Renderer::Device().CreateShader(entry.Source);
        if (!shader)
        {
            VtCoreError("ShaderLibrary: Reloading '{}' failed, keeping the "
                        "current shader: {}",
                        name, shader.Error());
            return false;
        }
        Replace(entry, *shader);
        VtCoreInfo("ShaderLibrary: Reloaded '{}'", name);
        return true;
    }

    bool ShaderLibrary::ReloadAll()
    {
        // Reload works on the map; collect the names first.
        Vector<String> names;
        for (auto& [name, entry] : m_Shaders)
            if (entry.Reloadable) names.PushBack(name);

        bool ok = true;
        for (const auto& name : names) ok &= Reload(name);
        return ok;
    }

    Vector<String> ShaderLibrary::Names() const
    {
        Vector<String> names;
        for (const auto& [name, entry] : m_Shaders) names.PushBack(name);
        Prism::Sort(names.begin(), names.end(),
                    [](const String& a, const String& b) { return a.Compare(b) < 0; });
        return names;
    }

    String ShaderLibrary::NameOf(const RHI::Shader& shader) const
    {
        for (const auto& [name, entry] : m_Shaders)
            if (entry.Shader.Raw() == &shader) return name;
        return String{};
    }

    Path ShaderLibrary::SourceFile(StringView name) const
    {
        auto it = m_Shaders.Find(String(name));
        if (it == m_Shaders.end() || !it->Value.Reloadable) return Path{};
        return it->Value.Source.File;
    }

    bool ShaderLibrary::IsReloadable(StringView name) const
    {
        auto it = m_Shaders.Find(String(name));
        return it != m_Shaders.end() && it->Value.Reloadable;
    }

    bool ShaderLibrary::Contains(StringView name) const
    {
        return m_Shaders.Contains(String(name));
    }

    ::Ref<RHI::Shader> ShaderLibrary::Get(StringView name) const
    {
        String key(name);
        VtCoreAssertFormat(m_Shaders.Contains(key),
                           "ShaderLibrary: No shader named '{}'", name);
        return m_Shaders.At(key).Shader;
    }

    ::Ref<RHI::Shader> ShaderLibrary::Find(StringView name) const
    {
        auto it = m_Shaders.Find(String(name));
        return it == m_Shaders.end() ? nullptr : it->Value.Shader;
    }
} // namespace Vortex
