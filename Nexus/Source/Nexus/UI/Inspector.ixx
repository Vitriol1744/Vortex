/*
 * Created by v1tr10l7 on 20.11.2026.
 * Copyright (c) 2024-2026, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
export module Nexus.Inspector;

import Prism.Memory.Ref;
import Vortex.RHI;

/**
 * Inspector panels for components and the assets they hold: scripts,
 * meshes, materials, shaders and textures. The Inspector window draws a
 * component's panel inside its tree node; the asset panels can be reused
 * anywhere (a content browser preview, a material editor...).
 */
export extern "C++"
{
    namespace Vortex
    {
        class Actor;
        class Material;
        class Mesh;
        struct MeshComponent;
        struct ScriptComponent;
    } // namespace Vortex

    namespace Vortex::Editor
    {
        class EditorContext;

        namespace Inspector
        {
            /// Class picker with status, the class's public fields (edited
            /// defaults in Edit mode, live values in Play), its source file
            /// and the script build.
            void ScriptComponentPanel(Actor& actor, ScriptComponent& script,
                                      EditorContext& context);

            /// The mesh asset and its stats, then its material(s).
            void MeshComponentPanel(Actor& actor, MeshComponent& component,
                                    EditorContext& context);

            /// A mesh asset's geometry: counts, bounds, submeshes, skeleton.
            void MeshPanel(Mesh& mesh);

            /// Shader, surface constants and every texture slot of a
            /// material. `owner`, when given, keeps its serialized texture in
            /// step with the albedo slot.
            void MaterialPanel(Material& material, const char* id,
                               MeshComponent* owner = nullptr);

            /// A shader's source, stages and reflected bindings.
            void ShaderPanel(RHI::Shader& shader);

            /// A texture's preview, size, format, mips and memory.
            void TexturePanel(const Prism::Ref<RHI::Texture>& texture);
        } // namespace Inspector
    } // namespace Vortex::Editor
}
