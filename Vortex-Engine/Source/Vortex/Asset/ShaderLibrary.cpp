/*
 * Created by v1tr10l7 on 07.06.2026.
 * Copyright (c) 2024-2026, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#include <Vortex/Asset/ShaderLibrary.hpp>

namespace Vortex
{
    ShaderLibrary::ShaderLibrary()  = default;
    ShaderLibrary::~ShaderLibrary() = default;

    void ShaderLibrary::Add(::Ref<Shader> shader)
    {
        const String& name = shader->Name();
        Add(name, shader);
    }
    void ShaderLibrary::Add(StringView name, ::Ref<Shader> shader)
    {
        if (m_Shaders.Contains(name))
            VtCoreWarn(
                "Shader with name '{}' already exists in the library! "
                "Overwriting.",
                name);
        m_Shaders[name] = shader;
    }

    void ShaderLibrary::Load(PathView path, bool forceCompile,
                             bool disableOptimization)
    {
        IgnoreUnused(forceCompile);
        IgnoreUnused(disableOptimization);
        ::Ref<Shader> shader = Shader::Create(path);
        Add(shader);
    }

    void ShaderLibrary::Load(StringView name, PathView path)
    {
        ::Ref<Shader> shader = Shader::Create(path);
        Add(name, shader);
    }

    const ::Ref<Shader>& ShaderLibrary::Get(StringView name) const
    {
        VtCoreAssertFormat(m_Shaders.Contains(name),
                           "Shader with name '{}' not found in the library!",
                           name);
        return m_Shaders.At(name);
    }
}; // namespace Vortex
