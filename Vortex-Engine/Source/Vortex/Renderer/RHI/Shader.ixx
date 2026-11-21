/*
 * Created by v1tr10l7 on 15.11.2026.
 * Copyright (c) 2024-2026, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
module;

#include <Vortex/Core/Types.hpp>
#include <Vortex/Renderer/RHI/Export.hpp>

export module Vortex.RHI.Shader;

export import Vortex.RHI.BindGroup;

import Prism.Containers.KeyValuePair;
import Prism.Containers.Vector;
import Prism.Memory.Ref;
import Prism.Memory.RefCounted;
import Prism.String.String;
import Prism.String.StringView;
import Prism.Utility.Path;

/**
 * Shaders are HLSL (or, for files ending in .glsl, GLSL), compiled to SPIR-V
 * at creation and reflected: the resulting ShaderLayout tells pipelines and
 * materials which bind groups and push constants the shader expects, so
 * nothing has to be declared twice.
 *
 * An HLSL file is one translation unit holding every stage; each stage is an
 * entry point named by a pragma:
 *
 *   #pragma vertex   VSMain
 *   #pragma fragment PSMain
 *   #include "include/Scene.hlsli"   // relative to the including file
 *
 * (stages: vertex, fragment, compute, geometry, tesscontrol, tesseval).
 * Resources are placed with [[vk::binding(binding, set)]], push constants
 * declared `[[vk::push_constant]] ConstantBuffer<T>`. A Texture* is bound
 * together with its sampler (a combined image sampler): the SamplerState its
 * Sample() call names is only syntax and is compiled out. Matrices use
 * HLSL's default column-major packing, which matches glm, so `mul(M, v)`
 * computes what GLSL's `M * v` does.
 *
 * A GLSL file holds every stage too, each starting with its own `#version`
 * followed by shaderc's `#pragma shader_stage(vertex|fragment|compute|...)`.
 */
export namespace Vortex::RHI
{
    enum class ShaderLanguage : u8
    {
        eHLSL,
        eGLSL,
    };

    struct ShaderSource
    {
        ShaderStage Stage = ShaderStage::eNone;
        String      Code{};
        /// Shown in compile errors; HLSL resolves #include relative to it.
        String      Name{};
        /// HLSL only: the function the stage starts in (GLSL's is main).
        String      EntryPoint{"main"};
    };

    struct ShaderDesc
    {
        /// A multi-stage file. Ignored when Sources isn't empty.
        Path                              File;
        /// Stages given directly as source code.
        Vector<ShaderSource>              Sources;
        /// Preprocessor definitions, `#define Key Value`.
        Vector<Prism::KeyValuePair<String, String>> Defines;
        String                            DebugName{};
        /// Language of Sources and File; a File ending in .glsl is GLSL
        /// whatever this says.
        ShaderLanguage                    Language = ShaderLanguage::eHLSL;
    };

    struct PushConstantRange
    {
        ShaderStage Stages = ShaderStage::eNone;
        u32         Offset = 0;
        u32         Size   = 0;
    };

    struct ShaderLayout
    {
        /// Indexed by set number; sets the shader doesn't use are empty.
        Vector<BindGroupLayoutDesc> Sets;
        Vector<PushConstantRange>   PushConstants;

        /// Finds a binding by its name in the shader (the block or
        /// ConstantBuffer name for buffers).
        inline const BindingLayoutEntry* Find(StringView name,
                                              u32*       outSet = nullptr) const
        {
            for (u32 set = 0; set < Sets.Size(); ++set)
            {
                if (auto* entry = Sets[set].Find(name))
                {
                    if (outSet) *outSet = set;
                    return entry;
                }
            }
            return nullptr;
        }
    };

    class VT_RENDERER_RHI_API Shader : public RefCounted<Shader>
    {
      public:
        Shader(const Shader&)            = delete;
        Shader& operator=(const Shader&) = delete;
        virtual ~Shader()                = default;

        inline StringView          Name() const { return m_Name; }
        inline ShaderStage         Stages() const { return m_Stages; }
        inline const ShaderLayout& Layout() const { return m_Layout; }

        /// Layout object of `set`, shared by every pipeline and bind group
        /// made from this shader. Null for a set the shader doesn't use.
        virtual ::Ref<RHI::BindGroupLayout> SetLayout(u32 set) const = 0;

      protected:
        Shader() = default;

        String       m_Name{};
        ShaderStage  m_Stages = ShaderStage::eNone;
        ShaderLayout m_Layout;
    };
} // namespace Vortex::RHI
