/*
 * Created by v1tr10l7 on 15.11.2026.
 * Copyright (c) 2024-2026, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
module;

#include <Vortex/Core/Types.hpp>
#include <Vortex/Renderer/Export.hpp>

export module Vortex.Renderer.Material;

export import Vortex.RHI;

import Prism.Containers.Vector;
import Prism.Memory.Ref;
import Prism.Memory.RefCounted;
import Prism.String.String;
import Prism.String.StringView;

/**
 * A shader plus the resources its material sets read, addressed by their GLSL
 * names:
 *
 *   auto material = Material::Create(shader);
 *   material->Set("texSampler", albedo);
 *   ...
 *   cmd.BindPipeline(*pipeline);
 *   material->Bind(cmd);
 *
 * Convention: set 0 holds per-frame/per-view data and is bound by the
 * renderer; sets 1 and up belong to the material. Unassigned texture slots
 * are filled with the white texture so a half-configured material still
 * renders; unassigned buffers are an error.
 *
 * The shader can be swapped at any time (SetShader); renderers draw a
 * material with the pipeline made for its current shader. A material whose
 * shader comes from the ShaderLibrary follows it when it is reloaded.
 */
export extern "C++"
{
    namespace Vortex
    {
        class VT_RENDERER_API Material : public RefCounted<Material>
        {
          public:
            /// Sets below `firstSet` are left to the renderer.
            static ::Ref<Material> Create(::Ref<RHI::Shader> shader,
                                          u32                firstSet = 1);

            explicit Material(::Ref<RHI::Shader> shader, u32 firstSet);
            Material(const Material&)            = delete;
            Material& operator=(const Material&) = delete;

            inline ::Ref<RHI::Shader> Shader() const { return m_Shader; }
            /// Switches to `shader`. Resources assigned so far carry over to
            /// the bindings of the same name and type; the new shader's other
            /// bindings start out at their defaults.
            void SetShader(::Ref<RHI::Shader> shader);

            /// Metal/roughness surface constants, pushed per draw by the scene
            /// renderer (the albedo texture multiplies Albedo).
            struct SurfaceParameters
            {
                f32 Albedo[4] = {1.0f, 1.0f, 1.0f, 1.0f};
                f32 Metallic  = 0.0f;
                f32 Roughness = 0.6f;
            };
            SurfaceParameters Surface;

            /// Binds a sampled texture. `sampler` defaults to the renderer's
            /// linear sampler. Returns false if the shader has no such
            /// material binding.
            bool Set(StringView name, ::Ref<RHI::Texture> texture,
                     ::Ref<RHI::Sampler> sampler = nullptr);
            /// Binds a uniform or storage buffer.
            bool Set(StringView name, ::Ref<RHI::Buffer> buffer);

            /// A binding of the material's sets and what it holds now.
            struct BindingInfo
            {
                String              Name{};
                RHI::BindingType    Type    = RHI::BindingType::eUniformBuffer;
                u32                 Set     = 0;
                u32                 Binding = 0;
                RHI::ShaderStage    Stages  = RHI::ShaderStage::eNone;
                ::Ref<RHI::Texture> Texture = nullptr;
                ::Ref<RHI::Buffer>  Buffer  = nullptr;
            };
            /// Every binding of the material's sets (the renderer's sets
            /// excluded), in set and binding order.
            Vector<BindingInfo> Bindings() const;

            /// One material set, as bound on the render thread.
            struct BoundSet
            {
                u32                   Set = 0;
                ::Ref<RHI::BindGroup> Group;
            };

            /// The material's bind groups, rebuilding those whose resources
            /// changed. Picks up a reloaded library shader first. Call on the
            /// thread that owns the material (the game thread); hand the
            /// result to the render thread.
            Vector<BoundSet> Resolve();
            /// Binds resolved sets (after BindPipeline with a pipeline made
            /// from the same shader).
            static void      Bind(RHI::CommandList& cmd, const Vector<BoundSet>& sets);
            /// Resolve + Bind, for materials only the render thread uses.
            void             Bind(RHI::CommandList& cmd);

          private:
            struct SetState
            {
                u32                         Index = 0;
                ::Ref<RHI::BindGroupLayout> Layout;
                Vector<RHI::BindGroupEntry> Entries;
                ::Ref<RHI::BindGroup>       Group;
                bool                        Dirty = true;
            };

            ::Ref<RHI::Shader> m_Shader;
            Vector<SetState>   m_Sets;
            u32                m_FirstSet = 1;
            /// ShaderLibrary generation last checked for a reloaded shader.
            u64                m_LibraryGeneration = 0;

            RHI::BindGroupEntry* Find(StringView name, RHI::BindingType* type);
            /// Default-filled sets of m_Shader.
            void                 BuildSets();
            /// Switches to the library's shader of the same name, if it was
            /// reloaded (game thread).
            void                 FollowLibrary();
            Vector<BoundSet>     ResolveSets();
        };
    } // namespace Vortex
}
