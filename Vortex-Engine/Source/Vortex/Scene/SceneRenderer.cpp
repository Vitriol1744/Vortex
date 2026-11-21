/*
 * Created by v1tr10l7 on 15.11.2026.
 * Copyright (c) 2024-2026, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#include <Vortex/Core/Profiler.hpp>
#include <Vortex/Renderer/Mesh.hpp>
#include <Vortex/Renderer/Renderer2D.hpp>
#include <Vortex/Renderer/ShaderLibrary.hpp>
#include <Vortex/Scene/Actor.hpp>
#include <Vortex/Scene/Camera.hpp>
#include <Vortex/Scene/Components.hpp>
#include <Vortex/Scene/Scene.hpp>
#include <Vortex/Scene/SceneRenderer.hpp>

#include <cstring>

import Prism.Algorithm.Sort;
import Prism.Containers.Vector;
import Prism.String.Formatter;
import Vortex.Renderer;
import Vortex.Renderer.Material;

namespace Vortex::SceneRenderer
{
    namespace
    {
        constexpr StringView kStaticShader  = "PBR";
        constexpr StringView kSkinnedShader = "PBR.Skinned";
        constexpr StringView kSkyboxShader  = "Skybox";
        constexpr StringView kGridShader    = "Grid";
        constexpr StringView kDefaultEnvironment = "assets/env/birchwood_4k.hdr";

        /// Mirrors `CameraData` in Scene.hlsli (std140).
        struct CameraData
        {
            Mat4       ViewProjection;
            Mat4       InverseViewProjection;
            Math::Vec4 Position;
            Math::Vec4 LightDirection;
            Math::Vec4 LightColor;
            Math::Vec4 Environment;
        };
        /// Mirrors `ObjectData` in pbr.hlsl.
        struct ObjectData
        {
            Mat4       Model;
            Math::Vec4 Albedo;
            Math::Vec4 Surface;
        };

        /// Per-view data of one frame slot.
        struct View
        {
            ::Ref<RHI::Buffer>    Camera;
            /// Every drawn object's ObjectData, indexed by instance.
            ::Ref<RHI::Buffer>    Objects;
            usize                 ObjectCapacity = 0;
            ::Ref<RHI::BindGroup> Lit;    // set 0 of the PBR shaders
            ::Ref<RHI::BindGroup> Skybox; // set 0 of the skybox shader
            ::Ref<RHI::BindGroup> Grid;   // set 0 of the grid shader
            RHI::Shader*          GridShader = nullptr;
            /// Environment and shaders the groups were made for.
            Environment*          Bound     = nullptr;
            RHI::Shader*          LitShader = nullptr;
            RHI::Shader*          SkyShader = nullptr;
        };
        struct PoseBuffer
        {
            ::Ref<RHI::Buffer>    Bones;
            ::Ref<RHI::BindGroup> Group;
            /// Shader whose set 2 layout the group was made with.
            RHI::Shader*          Shader = nullptr;
        };
        /// What one frame slot handed out; reset when the slot comes around.
        struct FramePool
        {
            Vector<View>       Views;
            Vector<PoseBuffer> Poses;
            usize              ViewsUsed = 0;
            usize              PosesUsed = 0;
            u64                Frame     = 0;
        };

        /// One mesh draw, captured on the game thread.
        struct DrawItem
        {
            ::Ref<Mesh>                Mesh;
            /// The material's shader (static meshes) or the skinned one.
            ::Ref<RHI::Shader>         Shader;
            /// Index into ViewSnapshot::Materials.
            u32                        Material = 0;
            ObjectData                 Object{};
            /// A range of the index buffer (a submesh); 0 = the whole mesh.
            u32                        FirstIndex = 0;
            u32                        IndexCount = 0;
            bool                       Skinned = false;
            Vector<Mat4>               Pose;
        };
        /// Everything a view needs, captured on the game thread.
        struct ViewSnapshot
        {
            CameraData         Camera{};
            ::Ref<Environment> Environment;
            /// The library's shaders as of this frame, so reloads apply to
            /// the next frame captured.
            ::Ref<RHI::Shader> LitShader;
            ::Ref<RHI::Shader> SkinnedShader;
            ::Ref<RHI::Shader> SkyboxShader;
            /// Null unless the view draws the grid.
            ::Ref<RHI::Shader> GridShader;
            Vector<DrawItem>   Draws;
            /// Each material drawn this frame, resolved once.
            Vector<Vector<Material::BoundSet>> Materials;
            Renderer2D::Batch  Sprites;
        };

        /// Fixed-function state of each kind of draw. The shader is filled
        /// in per draw and the pipeline comes from Renderer::Pipelines(), so
        /// any shader (a material's, a reloaded one) just works.
        RHI::GraphicsPipelineDesc s_StaticDesc;
        RHI::GraphicsPipelineDesc s_SkinnedDesc;
        RHI::GraphicsPipelineDesc s_SkyboxDesc;
        RHI::GraphicsPipelineDesc s_GridDesc;
        ::Ref<Environment>   s_Environment;
        Vector<FramePool>    s_Frames;
        Settings             s_Settings;

        FramePool& CurrentPool()
        {
            auto& device = Renderer::Device();
            auto& pool   = s_Frames[device.FrameIndex()];
            if (pool.Frame != device.FrameNumber())
            {
                pool.Frame     = device.FrameNumber();
                pool.ViewsUsed = 0;
                pool.PosesUsed = 0;
            }
            return pool;
        }

        ::Ref<RHI::BindGroup> MakeGroup(const RHI::Shader& shader, u32 set,
                                        Vector<RHI::BindGroupEntry> entries,
                                        StringView name)
        {
            RHI::BindGroupDesc desc{};
            desc.Layout    = shader.SetLayout(set);
            desc.Entries   = Move(entries);
            desc.DebugName = String(name);
            return Renderer::Device().CreateBindGroup(desc);
        }

        View& AcquireView(const ViewSnapshot& snapshot, usize objectCount)
        {
            auto& device = Renderer::Device();
            auto& pool   = CurrentPool();
            if (pool.ViewsUsed == pool.Views.Size())
            {
                RHI::BufferDesc desc{};
                desc.Size      = sizeof(CameraData);
                desc.Usage     = RHI::BufferUsage::eUniform;
                desc.Memory    = RHI::MemoryUsage::eCpuToGpu;
                desc.DebugName = "SceneRenderer: camera";
                pool.Views.EmplaceBack().Camera = device.CreateBuffer(desc);
            }
            View& view = pool.Views[pool.ViewsUsed++];

            // Grown (with headroom) when a frame draws more objects.
            bool resized = false;
            if (!view.Objects || view.ObjectCapacity < objectCount)
            {
                usize capacity = view.ObjectCapacity ? view.ObjectCapacity : 256;
                while (capacity < objectCount) capacity *= 2;
                RHI::BufferDesc desc{};
                desc.Size           = capacity * sizeof(ObjectData);
                desc.Usage          = RHI::BufferUsage::eStorage;
                desc.Memory         = RHI::MemoryUsage::eCpuToGpu;
                desc.DebugName      = "SceneRenderer: objects";
                view.Objects        = device.CreateBuffer(desc);
                view.ObjectCapacity = capacity;
                resized             = true;
            }

            // (Re)build the groups when the environment, a shader or the
            // object buffer changed.
            Environment* environment = snapshot.Environment.Raw();
            if (resized || view.Bound != environment
                || view.LitShader != snapshot.LitShader.Raw()
                || view.SkyShader != snapshot.SkyboxShader.Raw())
            {
                auto cube = Renderer::LinearClampSampler();
                view.Lit  = MakeGroup(
                    *snapshot.LitShader, 0,
                    {RHI::BindGroupEntry::Uniform(0, view.Camera),
                     RHI::BindGroupEntry::SampledTexture(1, environment->Irradiance(), cube),
                     RHI::BindGroupEntry::SampledTexture(2, environment->Prefiltered(), cube),
                     RHI::BindGroupEntry::SampledTexture(3, Environment::BRDFLut(), cube),
                     RHI::BindGroupEntry::Uniform(5, view.Objects)},
                    "SceneRenderer: view");
                view.Skybox = MakeGroup(
                    *snapshot.SkyboxShader, 0,
                    {RHI::BindGroupEntry::Uniform(0, view.Camera),
                     RHI::BindGroupEntry::SampledTexture(4, environment->Radiance(), cube)},
                    "SceneRenderer: skybox");
                view.Bound     = environment;
                view.LitShader = snapshot.LitShader.Raw();
                view.SkyShader = snapshot.SkyboxShader.Raw();
            }
            if (snapshot.GridShader && view.GridShader != snapshot.GridShader.Raw())
            {
                view.Grid       = MakeGroup(*snapshot.GridShader, 0,
                                            {RHI::BindGroupEntry::Uniform(0, view.Camera)},
                                            "SceneRenderer: grid");
                view.GridShader = snapshot.GridShader.Raw();
            }
            view.Camera->Write(&snapshot.Camera, sizeof(CameraData));
            return view;
        }

        RHI::BindGroup& AcquirePose(const Vector<Mat4>& pose, RHI::Shader& shader)
        {
            auto& pool = CurrentPool();
            if (pool.PosesUsed == pool.Poses.Size())
            {
                RHI::BufferDesc desc{};
                desc.Size      = kMaxBones * sizeof(Mat4);
                desc.Usage     = RHI::BufferUsage::eStorage;
                desc.Memory    = RHI::MemoryUsage::eCpuToGpu;
                desc.DebugName = "SceneRenderer: pose";
                PoseBuffer& buffer = pool.Poses.EmplaceBack();
                buffer.Bones       = Renderer::Device().CreateBuffer(desc);
            }
            PoseBuffer& buffer = pool.Poses[pool.PosesUsed++];
            if (buffer.Shader != &shader)
            {
                buffer.Group  = MakeGroup(shader, 2,
                                          {RHI::BindGroupEntry::Uniform(0, buffer.Bones)},
                                          "SceneRenderer: pose");
                buffer.Shader = &shader;
            }
            usize       count  = pose.Size() < kMaxBones ? pose.Size() : kMaxBones;
            buffer.Bones->Write(pose.Raw(), count * sizeof(Mat4));
            return *buffer.Group;
        }

        RHI::GraphicsPipelineDesc MeshDesc(bool skinned)
        {
            RHI::GraphicsPipelineDesc desc{};
            desc.VertexBindings.PushBack(skinned ? Mesh::SkinnedLayout()
                                                 : Mesh::StaticLayout());
            // Imported models aren't consistently wound; draw both faces.
            desc.Raster.Cull = RHI::CullMode::eNone;
            desc.ColorFormats.PushBack(Renderer::kSceneColorFormat);
            desc.DepthFormat = Renderer::kSceneDepthFormat;
            desc.DebugName   = skinned ? "SceneRenderer: skinned mesh"
                                       : "SceneRenderer: mesh";
            return desc;
        }

        /// The pipeline drawing `desc`'s state with `shader`.
        ::Ref<RHI::Pipeline> PipelineFor(RHI::GraphicsPipelineDesc& desc,
                                         const ::Ref<RHI::Shader>& shader)
        {
            desc.Shader = shader;
            auto pipeline = Renderer::Pipelines().Get(desc);
            // Don't keep the shader alive through the template.
            desc.Shader = nullptr;
            return pipeline;
        }

        bool LoadShaders()
        {
            auto& library = Renderer::ShaderLibrary();
            if (!library.Load(kStaticShader, "assets/shaders/pbr.hlsl")) return false;

            RHI::ShaderDesc skinned{};
            skinned.File = Path("assets/shaders/pbr.hlsl");
            skinned.Defines.PushBack({String("SKINNED"), String("1")});
            if (!library.Load(kSkinnedShader, skinned)) return false;
            if (!library.Load(kSkyboxShader, "assets/shaders/skybox.hlsl")) return false;
            return library.Load(kGridShader, "assets/shaders/grid.hlsl");
        }
    } // namespace

    bool Initialize()
    {
        VtProfileFunction();
        if (!LoadShaders()) return false;

        s_StaticDesc  = MeshDesc(false);
        s_SkinnedDesc = MeshDesc(true);

        RHI::GraphicsPipelineDesc skybox{};
        skybox.Raster.Cull = RHI::CullMode::eNone;
        // At the far plane, behind everything drawn before it.
        skybox.DepthStencil.DepthWrite   = false;
        skybox.DepthStencil.DepthCompare = RHI::CompareOp::eLessOrEqual;
        skybox.ColorFormats.PushBack(Renderer::kSceneColorFormat);
        skybox.DepthFormat = Renderer::kSceneDepthFormat;
        skybox.DebugName   = "SceneRenderer: skybox";
        s_SkyboxDesc       = skybox;

        RHI::GraphicsPipelineDesc grid{};
        grid.Raster.Cull = RHI::CullMode::eNone;
        // Tested against the scene, but transparent: writes no depth.
        grid.DepthStencil.DepthWrite   = false;
        grid.DepthStencil.DepthCompare = RHI::CompareOp::eLessOrEqual;
        grid.Blend.PushBack(RHI::BlendAttachment::AlphaBlend());
        grid.ColorFormats.PushBack(Renderer::kSceneColorFormat);
        grid.DepthFormat = Renderer::kSceneDepthFormat;
        grid.DebugName   = "SceneRenderer: grid";
        s_GridDesc       = grid;

        // Compile the default pipelines now rather than on the first frame.
        auto& library = Renderer::ShaderLibrary();
        (void)PipelineFor(s_StaticDesc, library.Get(kStaticShader));
        (void)PipelineFor(s_SkinnedDesc, library.Get(kSkinnedShader));
        (void)PipelineFor(s_SkyboxDesc, library.Get(kSkyboxShader));
        (void)PipelineFor(s_GridDesc, library.Get(kGridShader));

        s_Frames.Resize(Renderer::Device().FramesInFlight());
        // Baked now: GPU bakes run at load time, never inside a frame.
        Environment::BRDFLut();
        if (!LoadEnvironment(kDefaultEnvironment))
            s_Environment = Environment::Uniform(0.3f, 0.3f, 0.35f);

        return Renderer2D::Initialize(Renderer::kSceneColorFormat,
                                      Renderer::kSceneDepthFormat);
    }

    void Shutdown()
    {
        Renderer2D::Shutdown();
        s_Frames.Clear();
        s_Environment = nullptr;
        Environment::ReleaseShared();
        s_StaticDesc  = {};
        s_SkinnedDesc = {};
        s_SkyboxDesc  = {};
        s_GridDesc    = {};
    }

    Settings& GetSettings() { return s_Settings; }

    void SetEnvironment(::Ref<Environment> environment)
    {
        if (environment) s_Environment = environment;
    }
    ::Ref<Environment> GetEnvironment() { return s_Environment; }

    bool LoadEnvironment(PathView hdrPath)
    {
        auto environment = Environment::Load(hdrPath);
        if (!environment) return false;
        s_Environment = environment;
        return true;
    }

    ::Ref<RHI::Shader> Shader()
    {
        return Renderer::ShaderLibrary().Get(kStaticShader);
    }

    namespace
    {
        ViewSnapshot Capture(Scene& scene, Camera& camera, const ViewOptions& options)
        {
            VtProfileFunction();
            ViewSnapshot snapshot{};
            auto&        data          = snapshot.Camera;
            data.ViewProjection        = camera.ViewProjection();
            data.InverseViewProjection = glm::inverse(data.ViewProjection);
            data.Position              = Math::Vec4(camera.Position(), 1.0f);
            data.LightDirection
                = Math::Vec4(glm::normalize(s_Settings.LightDirection), 0.0f);
            data.LightColor = Math::Vec4(s_Settings.LightColor, s_Settings.LightIntensity);
            data.Environment
                = Math::Vec4(s_Settings.Exposure, s_Settings.EnvironmentIntensity,
                             s_Settings.SkyboxBlur, 0.0f);
            snapshot.Environment = s_Environment;

            auto& library          = Renderer::ShaderLibrary();
            snapshot.LitShader     = library.Get(kStaticShader);
            snapshot.SkinnedShader = library.Get(kSkinnedShader);
            snapshot.SkyboxShader  = library.Get(kSkyboxShader);
            if (options.DrawGrid) snapshot.GridShader = library.Get(kGridShader);

            auto& registry = scene.Registry();
            auto  meshes   = registry.view<MeshComponent>();

            // Many draws share a material (bullet holes, props, the level's
            // submeshes): resolve each once per frame.
            struct ResolvedMaterial
            {
                Material*          Source = nullptr;
                ::Ref<RHI::Shader> Shader;
            };
            Vector<ResolvedMaterial> resolved;
            // Index of `material` in snapshot.Materials (and `resolved`).
            auto resolve = [&](Material& material) -> u32
            {
                for (u32 i = 0; i < resolved.Size(); ++i)
                    if (resolved[i].Source == &material) return i;
                auto& entry  = resolved.EmplaceBack();
                entry.Source = &material;
                // Resolve first: it may switch to a reloaded shader.
                snapshot.Materials.PushBack(material.Resolve());
                entry.Shader = material.Shader();
                return static_cast<u32>(resolved.Size() - 1);
            };
            auto surfaceOf = [](const Material& material, ObjectData& object)
            {
                const auto& surface = material.Surface;
                object.Albedo  = Math::Vec4(surface.Albedo[0], surface.Albedo[1],
                                            surface.Albedo[2], surface.Albedo[3]);
                object.Surface = Math::Vec4(surface.Metallic, surface.Roughness, 0.0f, 0.0f);
            };

            usize expected = 0;
            for (auto entity : meshes)
            {
                const auto& component = meshes.get<MeshComponent>(entity);
                if (!component.Mesh) continue;
                expected += component.SubmeshMaterials.Empty()
                              ? 1 : component.Mesh->Submeshes().Size();
            }
            snapshot.Draws.Reserve(expected);

            for (auto entity : meshes)
            {
                auto& component = meshes.get<MeshComponent>(entity);
                if (!component.Mesh || !component.Material) continue;

                const auto* transform = registry.try_get<TransformComponent>(entity);
                Mat4        model     = transform ? Mat4(*transform) : Mat4(1.0f);

                bool skinned   = false;
                const auto* animation = registry.try_get<AnimationComponent>(entity);
                if (animation)
                    skinned = component.Mesh->IsSkinned() && !animation->Pose.Empty();

                // Multi-material models: one draw per submesh with its own
                // material (static meshes; skinned ones keep one material).
                if (!component.SubmeshMaterials.Empty() && !skinned)
                {
                    for (const auto& submesh : component.Mesh->Submeshes())
                    {
                        const auto& material
                            = submesh.MaterialIndex < component.SubmeshMaterials.Size()
                                ? component.SubmeshMaterials[submesh.MaterialIndex]
                                : component.Material;
                        if (!material || submesh.IndexCount == 0) continue;
                        u32       index   = resolve(*material);
                        DrawItem& part    = snapshot.Draws.EmplaceBack();
                        part.Mesh         = component.Mesh;
                        part.Material     = index;
                        part.Shader       = resolved[index].Shader;
                        part.Object.Model = model;
                        part.FirstIndex   = submesh.BaseIndex;
                        part.IndexCount   = submesh.IndexCount;
                        surfaceOf(*material, part.Object);
                    }
                    continue;
                }

                u32       index   = resolve(*component.Material);
                DrawItem& draw    = snapshot.Draws.EmplaceBack();
                draw.Mesh         = component.Mesh;
                draw.Material     = index;
                // Materials describe surfaces of static meshes; skinning
                // needs the skinned variant.
                draw.Skinned      = skinned;
                draw.Shader       = skinned ? snapshot.SkinnedShader : resolved[index].Shader;
                if (skinned) draw.Pose = animation->Pose;
                draw.Object.Model = model;
                surfaceOf(*component.Material, draw.Object);
            }

            Renderer2D::Begin();
            auto sprites = scene.Registry().group<TransformComponent>(
                entt::get<SpriteComponent>);
            for (auto entity : sprites)
            {
                const auto& [transform, sprite]
                    = sprites.get<TransformComponent, SpriteComponent>(entity);
                Renderer2D::DrawQuad(transform, sprite.Color);
            }
            snapshot.Sprites = Renderer2D::End();
            return snapshot;
        }

        /// Render thread: rendering has begun on the view's target.
        void Record(RHI::CommandList& cmd, const ViewSnapshot& snapshot)
        {
            VtProfileFunction();
            // Draws are sorted by shader, material, mesh and index range,
            // so state is only rebound when it changes, and runs of objects
            // drawn alike (bullet holes, crates) become one instanced draw.
            auto materialKey = [](const DrawItem& draw) { return draw.Material; };
            Vector<u32> order;
            order.Reserve(snapshot.Draws.Size());
            for (u32 i = 0; i < snapshot.Draws.Size(); ++i) order.PushBack(i);
            Prism::Sort(order.begin(), order.end(),
                        [&](u32 lhs, u32 rhs)
                        {
                            const auto& a = snapshot.Draws[lhs];
                            const auto& b = snapshot.Draws[rhs];
                            if (a.Skinned != b.Skinned) return !a.Skinned;
                            if (a.Shader.Raw() != b.Shader.Raw())
                                return a.Shader.Raw() < b.Shader.Raw();
                            if (materialKey(a) != materialKey(b))
                                return materialKey(a) < materialKey(b);
                            if (a.Mesh.Raw() != b.Mesh.Raw())
                                return a.Mesh.Raw() < b.Mesh.Raw();
                            return a.FirstIndex < b.FirstIndex;
                        });

            // Objects in draw order: a run's instances are contiguous.
            View& view = AcquireView(snapshot, order.Size());
            {
                Vector<ObjectData> objects;
                objects.Reserve(order.Size());
                for (u32 index : order) objects.PushBack(snapshot.Draws[index].Object);
                if (!objects.Empty())
                    view.Objects->Write(objects.Raw(), objects.Size() * sizeof(ObjectData));
            }
            auto sameBatch = [&](const DrawItem& a, const DrawItem& b)
            {
                return !a.Skinned && !b.Skinned && a.Shader.Raw() == b.Shader.Raw()
                    && materialKey(a) == materialKey(b) && a.Mesh.Raw() == b.Mesh.Raw()
                    && a.FirstIndex == b.FirstIndex && a.IndexCount == b.IndexCount;
            };

            RHI::Shader* boundShader   = nullptr;
            u32          boundMaterial = ~0u;
            const Mesh*  boundMesh     = nullptr;
            for (u32 first = 0; first < order.Size();)
            {
                const auto& draw = snapshot.Draws[order[first]];
                u32         count = 1;
                while (first + count < order.Size()
                       && sameBatch(draw, snapshot.Draws[order[first + count]]))
                    ++count;

                if (boundShader != draw.Shader.Raw())
                {
                    auto& state = draw.Skinned ? s_SkinnedDesc : s_StaticDesc;
                    cmd.BindPipeline(*PipelineFor(state, draw.Shader));
                    // Set 0 is the renderer's, laid out alike by every
                    // surface shader.
                    cmd.BindGroup(0, *view.Lit);
                    boundShader   = draw.Shader.Raw();
                    boundMaterial = ~0u;
                }
                if (draw.Skinned)
                    cmd.BindGroup(2, AcquirePose(draw.Pose, *draw.Shader));

                if (boundMaterial != draw.Material)
                {
                    Material::Bind(cmd, snapshot.Materials[draw.Material]);
                    boundMaterial = materialKey(draw);
                }
                cmd.PushConstants(RHI::ShaderStage::eVertex, first);
                if (boundMesh != draw.Mesh.Raw())
                {
                    cmd.BindVertexBuffer(0, *draw.Mesh->VertexBuffer());
                    cmd.BindIndexBuffer(*draw.Mesh->IndexBuffer(), RHI::IndexType::eU32);
                    boundMesh = draw.Mesh.Raw();
                }
                if (draw.IndexCount)
                    cmd.DrawIndexed(draw.IndexCount, count, draw.FirstIndex);
                else cmd.DrawIndexed(draw.Mesh->IndexCount(), count);
                first += count;
            }

            cmd.BindPipeline(*PipelineFor(s_SkyboxDesc, snapshot.SkyboxShader));
            cmd.BindGroup(0, *view.Skybox);
            cmd.Draw(3);

            if (snapshot.GridShader && view.Grid)
            {
                cmd.BindPipeline(*PipelineFor(s_GridDesc, snapshot.GridShader));
                cmd.BindGroup(0, *view.Grid);
                cmd.Draw(3);
            }

            Renderer2D::Draw(cmd, snapshot.Sprites, view.Camera);
        }
    } // namespace

    void AddToGraph(RenderGraph& graph, Scene& scene, Camera& camera,
                    SceneRenderTarget& target, const ViewOptions& options)
    {
        VtProfileFunction();
        struct PassData
        {
            RGTexture Color;
            RGTexture Depth;
        };
        auto color = graph.Import(target.Color(), RHI::ResourceState::eShaderRead);
        auto depth = graph.Import(target.Depth(), RHI::ResourceState::eDepthStencilWrite);

        graph.AddPass<PassData>(
            "Scene",
            [&](RGBuilder& builder, PassData& data)
            {
                // The skybox covers every pixel nothing else did.
                data.Color = builder.WriteColor(color, RHI::LoadOp::eDontCare);
                data.Depth = builder.WriteDepth(depth, RHI::LoadOp::eClear);
            },
            [snapshot = Capture(scene, camera, options)](const PassData&, RGContext& context)
            { Record(context.Cmd(), snapshot); });
    }
} // namespace Vortex::SceneRenderer
