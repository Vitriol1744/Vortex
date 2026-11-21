/*
 * Created by v1tr10l7 on 24.05.2026.
 * Copyright (c) 2024-2026, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#include <Vortex/Core/Profiler.hpp>
#include <Vortex/Renderer/Renderer2D.hpp>
#include <Vortex/Renderer/ShaderLibrary.hpp>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

import Prism.Containers.UnorderedMap;
import Prism.Containers.Vector;
import Vortex.Renderer;
import Vortex.Renderer.Material;

namespace Vortex::Renderer2D
{
    namespace
    {


        constexpr u32 kMaxQuads    = 20000;
        constexpr u32 kMaxVertices = kMaxQuads * 4;
        constexpr u32 kMaxIndices  = kMaxQuads * 6;

        /// Vertex memory of one frame slot; several flushes a frame append.
        struct FrameBuffer
        {
            ::Ref<RHI::Buffer> Buffer;
            u32                Used  = 0;
            u64                Frame = 0;
        };

        constexpr StringView kShader = "Sprite";

        /// Fixed-function state; the shader is the batch's (render thread).
        RHI::GraphicsPipelineDesc s_Desc;
        struct CameraGroup
        {
            ::Ref<RHI::BindGroup> Group;
            /// Shader whose set 0 layout the group was made with.
            RHI::Shader*          Shader = nullptr;
        };
        /// Camera bind groups, one per camera buffer seen (render thread).
        UnorderedMap<upointer, CameraGroup> s_CameraGroups;
        ::Ref<Material>      s_Material; // game thread
        ::Ref<RHI::Buffer>   s_IndexBuffer;
        Vector<FrameBuffer>  s_Frames;
        Vector<Vertex>       s_Vertices; // game thread
        bool                 s_Initialized = false;
    } // namespace

    bool Initialize(RHI::Format colorFormat, RHI::Format depthFormat)
    {
        if (s_Initialized) return true;
        auto& device  = Renderer::Device();
        auto& library = Renderer::ShaderLibrary();
        if (!library.Load(kShader, "assets/shaders/texture.hlsl")) return false;

        RHI::GraphicsPipelineDesc pipeline{};
        pipeline.VertexBindings.PushBack(RHI::VertexBinding::Packed(
            {RHI::Format::eRGB32Float, RHI::Format::eRGB32Float,
             RHI::Format::eRG32Float}));
        pipeline.Raster.Cull = RHI::CullMode::eNone;
        // Sprites are transparent: test against the scene, don't write.
        pipeline.DepthStencil.DepthWrite = false;
        pipeline.Blend.PushBack(RHI::BlendAttachment::AlphaBlend());
        pipeline.ColorFormats.PushBack(colorFormat);
        pipeline.DepthFormat = depthFormat;
        pipeline.DebugName   = "Renderer2D: quads";
        s_Desc               = pipeline;
        s_Material           = Material::Create(library.Get(kShader));

        // Compile the pipeline now rather than on the first sprite.
        pipeline.Shader = s_Material->Shader();
        (void)Renderer::Pipelines().Get(pipeline);

        Vector<u32> indices(kMaxIndices);
        for (u32 i = 0, vertex = 0; i < kMaxIndices; i += 6, vertex += 4)
        {
            indices[i + 0] = vertex + 0;
            indices[i + 1] = vertex + 1;
            indices[i + 2] = vertex + 2;
            indices[i + 3] = vertex + 2;
            indices[i + 4] = vertex + 3;
            indices[i + 5] = vertex + 0;
        }
        RHI::BufferDesc indexDesc{};
        indexDesc.Size      = kMaxIndices * sizeof(u32);
        indexDesc.Usage     = RHI::BufferUsage::eIndex;
        indexDesc.DebugName = "Renderer2D: indices";
        s_IndexBuffer       = device.CreateBuffer(indexDesc, indices.Raw());

        s_Frames.Resize(device.FramesInFlight());
        for (auto& frame : s_Frames)
        {
            RHI::BufferDesc desc{};
            desc.Size      = kMaxVertices * sizeof(Vertex);
            desc.Usage     = RHI::BufferUsage::eVertex;
            desc.Memory    = RHI::MemoryUsage::eCpuToGpu;
            desc.DebugName = "Renderer2D: vertices";
            frame.Buffer   = device.CreateBuffer(desc);
        }
        s_Vertices.Reserve(1024);
        s_Initialized = true;
        return true;
    }

    void Shutdown()
    {
        if (!s_Initialized) return;
        s_Frames.Clear();
        s_Vertices.Clear();
        s_CameraGroups.Clear();
        s_IndexBuffer = nullptr;
        s_Material    = nullptr;
        s_Desc        = {};
        s_Initialized = false;
    }

    void Begin() { s_Vertices.Clear(); }

    void DrawQuad(const Mat4& transform, const Math::Vec4& color)
    {
        constexpr Math::Vec2 texCoords[]
            = {{1.0f, 0.0f}, {0.0f, 0.0f}, {0.0f, 1.0f}, {1.0f, 1.0f}};
        constexpr Math::Vec4 positions[] = {{-0.5f, -0.5f, 0.0f, 1.0f},
                                            {0.5f, -0.5f, 0.0f, 1.0f},
                                            {0.5f, 0.5f, 0.0f, 1.0f},
                                            {-0.5f, 0.5f, 0.0f, 1.0f}};
        for (u32 i = 0; i < 4; ++i)
            s_Vertices.PushBack({Math::Vec3(transform * positions[i]),
                                 Math::Vec3(color), texCoords[i]});
    }

    Batch End()
    {
        Batch batch;
        batch.Vertices = Move(s_Vertices);
        s_Vertices     = {};
        if (batch.Vertices.Empty()) return batch;
        // Resolve first: it may switch to a reloaded shader.
        batch.Material = s_Material->Resolve();
        batch.Shader   = s_Material->Shader();
        return batch;
    }

    void Draw(RHI::CommandList& cmd, const Batch& batch,
              const ::Ref<RHI::Buffer>& camera)
    {
        if (batch.Empty()) return;
        VtProfileFunction();

        auto&        device = Renderer::Device();
        FrameBuffer& frame  = s_Frames[device.FrameIndex()];
        if (frame.Frame != device.FrameNumber())
        {
            frame.Frame = device.FrameNumber();
            frame.Used  = 0;
        }

        u32 count = batch.Vertices.Size();
        if (frame.Used + count > kMaxVertices)
        {
            VtCoreWarn("Renderer2D: More than {} quads this frame; dropping {}",
                       kMaxQuads, (frame.Used + count - kMaxVertices) / 4);
            count = kMaxVertices - frame.Used;
        }
        if (count == 0) return;

        usize offset = frame.Used * sizeof(Vertex);
        frame.Buffer->Write(batch.Vertices.Raw(), count * sizeof(Vertex), offset);
        frame.Used += count;

        struct
        {
            Mat4 Model{1.0f};
        } object;
        auto& group = s_CameraGroups[reinterpret_cast<upointer>(camera.Raw())];
        if (!group.Group || group.Shader != batch.Shader.Raw())
        {
            RHI::BindGroupDesc desc{};
            desc.Layout = batch.Shader->SetLayout(0);
            desc.Entries.PushBack(RHI::BindGroupEntry::Uniform(0, camera));
            desc.DebugName = "Renderer2D: camera";
            group.Group    = device.CreateBindGroup(desc);
            group.Shader   = batch.Shader.Raw();
        }

        s_Desc.Shader = batch.Shader;
        auto pipeline = Renderer::Pipelines().Get(s_Desc);
        s_Desc.Shader = nullptr;

        cmd.BindPipeline(*pipeline);
        cmd.BindGroup(0, *group.Group);
        Material::Bind(cmd, batch.Material);
        cmd.PushConstants(RHI::ShaderStage::eVertex, object);
        cmd.BindVertexBuffer(0, *frame.Buffer, offset);
        cmd.BindIndexBuffer(*s_IndexBuffer, RHI::IndexType::eU32);
        cmd.DrawIndexed(count / 4 * 6);
    }
}; // namespace Vortex::Renderer2D
