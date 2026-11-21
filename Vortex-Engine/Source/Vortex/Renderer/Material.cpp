/*
 * Created by v1tr10l7 on 15.11.2026.
 * Copyright (c) 2024-2026, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#include <Vortex/Core/Assertions.hpp>
#include <Vortex/Core/Types.hpp>
#include <Vortex/Renderer/ShaderLibrary.hpp>

import Vortex.Renderer;
import Vortex.Renderer.Material;
import Prism.String.Formatter;

namespace Vortex
{
    ::Ref<Material> Material::Create(::Ref<RHI::Shader> shader, u32 firstSet)
    {
        return CreateRef<Material>(shader, firstSet);
    }

    Material::Material(::Ref<RHI::Shader> shader, u32 firstSet)
        : m_Shader(shader)
        , m_FirstSet(firstSet)
    {
        VtCoreAssertMsg(m_Shader, "Material: Null shader");
        m_LibraryGeneration = Renderer::ShaderLibrary().Generation();
        BuildSets();
    }

    void Material::BuildSets()
    {
        m_Sets.Clear();
        const auto& sets = m_Shader->Layout().Sets;
        for (u32 index = m_FirstSet; index < sets.Size(); ++index)
        {
            if (sets[index].Entries.Empty()) continue;

            SetState& state = m_Sets.EmplaceBack();
            state.Index     = index;
            state.Layout    = m_Shader->SetLayout(index);
            // One entry per binding, prefilled so textures always have
            // something valid bound.
            for (const auto& binding : sets[index].Entries)
            {
                RHI::BindGroupEntry& entry = state.Entries.EmplaceBack();
                entry.Binding              = binding.Binding;
                if (binding.Type == RHI::BindingType::eSampledTexture
                    || binding.Type == RHI::BindingType::eTexture)
                    entry.Texture = Renderer::WhiteTexture();
                if (binding.Type == RHI::BindingType::eSampledTexture
                    || binding.Type == RHI::BindingType::eSampler)
                    entry.Sampler = Renderer::LinearSampler();
            }
        }
    }

    void Material::SetShader(::Ref<RHI::Shader> shader)
    {
        VtCoreAssertMsg(shader, "Material: Null shader");
        if (shader == m_Shader) return;

        Vector<SetState> previous = Move(m_Sets);
        m_Sets                    = {};
        m_Shader                  = Move(shader);
        BuildSets();

        // Carry resources over by binding name (and type: a texture can't
        // feed a buffer binding).
        for (const auto& old : previous)
        {
            for (const auto& oldEntry : old.Entries)
            {
                const auto* oldBinding = old.Layout->Desc().Find(oldEntry.Binding);
                if (!oldBinding) continue;
                for (auto& state : m_Sets)
                {
                    const auto* binding = state.Layout->Desc().Find(
                        StringView(oldBinding->Name));
                    if (!binding || binding->Type != oldBinding->Type) continue;
                    for (auto& entry : state.Entries)
                    {
                        if (entry.Binding != binding->Binding) continue;
                        u32 index    = entry.Binding;
                        entry        = oldEntry;
                        entry.Binding = index;
                    }
                }
            }
        }
    }

    void Material::FollowLibrary()
    {
        auto& library = Renderer::ShaderLibrary();
        if (m_LibraryGeneration == library.Generation()) return;
        m_LibraryGeneration = library.Generation();
        auto latest         = library.Find(m_Shader->Name());
        if (latest && latest != m_Shader) SetShader(latest);
    }

    RHI::BindGroupEntry* Material::Find(StringView name, RHI::BindingType* type)
    {
        for (auto& state : m_Sets)
        {
            const auto* binding = state.Layout->Desc().Find(name);
            if (!binding) continue;
            for (auto& entry : state.Entries)
            {
                if (entry.Binding != binding->Binding) continue;
                *type       = binding->Type;
                state.Dirty = true;
                return &entry;
            }
        }
        return nullptr;
    }

    bool Material::Set(StringView name, ::Ref<RHI::Texture> texture,
                       ::Ref<RHI::Sampler> sampler)
    {
        RHI::BindingType type{};
        auto*            entry = Find(name, &type);
        if (!entry)
        {
            VtCoreWarn("Material: Shader '{}' has no material binding '{}'",
                       m_Shader->Name(), name);
            return false;
        }
        VtCoreAssertFormat(type == RHI::BindingType::eSampledTexture
                               || type == RHI::BindingType::eTexture,
                           "Material: '{}' isn't a texture binding", name);
        entry->Texture = texture ? texture : Renderer::WhiteTexture();
        if (type == RHI::BindingType::eSampledTexture)
            entry->Sampler = sampler ? sampler : Renderer::LinearSampler();
        return true;
    }

    bool Material::Set(StringView name, ::Ref<RHI::Buffer> buffer)
    {
        RHI::BindingType type{};
        auto*            entry = Find(name, &type);
        if (!entry)
        {
            VtCoreWarn("Material: Shader '{}' has no material binding '{}'",
                       m_Shader->Name(), name);
            return false;
        }
        VtCoreAssertFormat(RHI::IsBufferBinding(type),
                           "Material: '{}' isn't a buffer binding", name);
        entry->Buffer = buffer;
        return true;
    }

    Vector<Material::BindingInfo> Material::Bindings() const
    {
        Vector<BindingInfo> bindings;
        for (const auto& state : m_Sets)
        {
            for (const auto& entry : state.Entries)
            {
                const auto* layout = state.Layout->Desc().Find(entry.Binding);
                if (!layout) continue;
                BindingInfo& info = bindings.EmplaceBack();
                info.Name    = layout->Name;
                info.Type    = layout->Type;
                info.Set     = state.Index;
                info.Binding = entry.Binding;
                info.Stages  = layout->Stages;
                info.Texture = entry.Texture;
                info.Buffer  = entry.Buffer;
            }
        }
        return bindings;
    }

    Vector<Material::BoundSet> Material::Resolve()
    {
        FollowLibrary();
        return ResolveSets();
    }

    Vector<Material::BoundSet> Material::ResolveSets()
    {
        Vector<BoundSet> sets;
        for (auto& state : m_Sets)
        {
            if (state.Dirty || !state.Group)
            {
                for (const auto& entry : state.Entries)
                {
                    const auto* binding = state.Layout->Desc().Find(entry.Binding);
                    VtCoreAssertFormat(!RHI::IsBufferBinding(binding->Type)
                                           || entry.Buffer,
                                       "Material: Buffer '{}' of shader '{}' "
                                       "was never set",
                                       binding->Name, m_Shader->Name());
                }
                RHI::BindGroupDesc desc{};
                desc.Layout    = state.Layout;
                desc.Entries   = state.Entries;
                desc.DebugName = Prism::Format("Material set {} ({})", state.Index,
                                               m_Shader->Name());
                // The previous group is freed once the GPU is done with it.
                state.Group = Renderer::Device().CreateBindGroup(desc);
                state.Dirty = false;
            }
            sets.PushBack({state.Index, state.Group});
        }
        return sets;
    }

    void Material::Bind(RHI::CommandList& cmd, const Vector<BoundSet>& sets)
    {
        for (const auto& set : sets) cmd.BindGroup(set.Set, *set.Group);
    }

    // May run on the render thread: no library lookups.
    void Material::Bind(RHI::CommandList& cmd) { Bind(cmd, ResolveSets()); }
} // namespace Vortex
