/*
 * Created by v1tr10l7 on 15.11.2026.
 * Copyright (c) 2024-2026, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#include <Vortex/Core/Profiler.hpp>
#include <Vortex/Renderer/ImGui/ImGuiLayer.hpp>

#include <cstddef>
#include <cstring>

import Prism.Containers.Span;
import Prism.String.Formatter;
import Prism.Utility.Atomic;
import Vortex.Renderer;

namespace Vortex
{
    using Backend = ImGuiLayer::VortexBackend;

    namespace
    {
        Backend* s_Backend = nullptr;

        constexpr const char* kVertexShader = R"(
struct TransformData
{
    float2 Scale;
    float2 Translate;
};
[[vk::push_constant]] ConstantBuffer<TransformData> Transform;

struct VSInput
{
    [[vk::location(0)]] float2 Position : POSITION;
    [[vk::location(1)]] float2 UV : TEXCOORD0;
    [[vk::location(2)]] float4 Color : COLOR0;
};

struct VSOutput
{
    float4                     Position : SV_Position;
    [[vk::location(0)]] float4 Color : COLOR0;
    [[vk::location(1)]] float2 UV : TEXCOORD0;
};

VSOutput main(VSInput input)
{
    VSOutput output;
    output.Color    = input.Color;
    output.UV       = input.UV;
    output.Position = float4(input.Position * Transform.Scale + Transform.Translate, 0.0, 1.0);
    return output;
}
)";
        constexpr const char* kFragmentShader = R"(
[[vk::binding(0, 0)]] Texture2D Atlas;
SamplerState Sampler;

struct PSInput
{
    float4                     Position : SV_Position;
    [[vk::location(0)]] float4 Color : COLOR0;
    [[vk::location(1)]] float2 UV : TEXCOORD0;
};

float4 main(PSInput input) : SV_Target0
{
    return input.Color * Atlas.Sample(Sampler, input.UV);
}
)";

        struct PushConstants
        {
            f32 Scale[2];
            f32 Translate[2];
        };

        template <typename T>
        Vector<T> CopyOf(const ImVector<T>& source)
        {
            Vector<T> copy(static_cast<usize>(source.Size));
            if (source.Size)
                std::memcpy(copy.Raw(), source.Data, sizeof(T) * source.Size);
            return copy;
        }
    } // namespace

    //------------------------------------------------------------------------
    // Lifetime --------------------------------------------------------------
    //------------------------------------------------------------------------

    Backend::VortexBackend()
    {
        VtCoreAssertMsg(!s_Backend, "ImGui: VortexBackend already exists");
        s_Backend = this;

        ImGuiIO& io                = ImGui::GetIO();
        io.BackendRendererName     = "Vortex RHI";
        io.BackendRendererUserData = this;
        io.BackendFlags |= ImGuiBackendFlags_RendererHasVtxOffset
                         | ImGuiBackendFlags_RendererHasTextures
                         | ImGuiBackendFlags_RendererHasViewports;

        auto& device = Renderer::Device();

        RHI::ShaderDesc shader{};
        shader.DebugName = "ImGui";
        shader.Sources.PushBack(
            {RHI::ShaderStage::eVertex, String(kVertexShader), String("ImGui.vertex.hlsl")});
        shader.Sources.PushBack({RHI::ShaderStage::eFragment,
                                 String(kFragmentShader), String("ImGui.fragment.hlsl")});
        auto result = device.CreateShader(shader);
        VtCoreAssertFormat(result, "ImGui: {}", result ? String() : result.Error());
        m_Shader = *result;

        m_MainViewport.Frames.Resize(device.FramesInFlight());

        ImGuiPlatformIO& platform       = ImGui::GetPlatformIO();
        platform.Renderer_CreateWindow  = CreateWindow;
        platform.Renderer_DestroyWindow = DestroyWindow;
        platform.Renderer_SetWindowSize = SetWindowSize;
        platform.Renderer_RenderWindow  = RenderWindow;
        platform.Renderer_SwapBuffers   = [](ImGuiViewport*, void*) {};
        // Bounded by the largest texture the device can create.
        platform.Renderer_TextureMaxWidth = platform.Renderer_TextureMaxHeight
            = static_cast<int>(device.Limits().MaxTextureDimension2D);
    }

    Backend::~VortexBackend()
    {
        // The render thread may still draw with this backend's resources.
        Renderer::Flush();

        // Textures ImGui still owns (the font atlas): mark them destroyed so
        // a later backend recreates them.
        for (ImTextureData* texture : ImGui::GetPlatformIO().Textures)
        {
            if (texture->RefCount != 1) continue;
            texture->SetTexID(ImTextureID_Invalid);
            texture->SetStatus(ImTextureStatus_Destroyed);
            texture->BackendUserData = nullptr;
        }
        m_Textures.Clear();
        // Its pipelines would outlive ImGui in the renderer's cache.
        if (m_Shader) Renderer::Pipelines().Invalidate(*m_Shader);
        m_MainViewport = {};
        m_Shader       = nullptr;

        ImGuiIO& io                = ImGui::GetIO();
        io.BackendRendererName     = nullptr;
        io.BackendRendererUserData = nullptr;
        io.BackendFlags &= ~(ImGuiBackendFlags_RendererHasVtxOffset
                             | ImGuiBackendFlags_RendererHasTextures
                             | ImGuiBackendFlags_RendererHasViewports);
        ImGuiPlatformIO& platform       = ImGui::GetPlatformIO();
        platform.Renderer_CreateWindow  = nullptr;
        platform.Renderer_DestroyWindow = nullptr;
        platform.Renderer_SetWindowSize = nullptr;
        platform.Renderer_RenderWindow  = nullptr;
        platform.Renderer_SwapBuffers   = nullptr;
        s_Backend                       = nullptr;
    }

    Backend& Backend::Get()
    {
        VtCoreAssertMsg(s_Backend, "ImGui: No VortexBackend (ImGuiLayer not attached)");
        return *s_Backend;
    }

    //------------------------------------------------------------------------
    // Textures (game thread) ------------------------------------------------
    //------------------------------------------------------------------------

    ImTextureID Backend::TextureID(const ::Ref<RHI::Texture>& texture)
    {
        if (!texture) return ImTextureID_Invalid;
        upointer key     = reinterpret_cast<upointer>(texture.Raw());
        auto&    binding = m_Textures[key];
        if (!binding.Texture)
        {
            binding.Texture = texture;
            RHI::BindGroupDesc desc{};
            desc.Layout = m_Shader->SetLayout(0);
            desc.Entries.PushBack(RHI::BindGroupEntry::SampledTexture(
                0, texture, Renderer::LinearSampler()));
            desc.DebugName = "ImGui texture";
            binding.Group  = Renderer::Device().CreateBindGroup(desc);
        }
        binding.LastUsed = Renderer::FrameNumber();
        return static_cast<ImTextureID>(key);
    }

    ::Ref<RHI::BindGroup> Backend::BindingFor(ImTextureID id)
    {
        auto it = m_Textures.Find(static_cast<upointer>(id));
        // Unknown IDs (stale or never registered) show white.
        if (it == m_Textures.end())
            return BindingFor(TextureID(Renderer::WhiteTexture()));
        it->Value.LastUsed = Renderer::FrameNumber();
        return it->Value.Group;
    }

    void Backend::CollectUnusedTextures()
    {
        // User textures nobody drew for a while are released; draws already
        // captured keep their bind groups (and so their textures) alive.
        u64 frame = Renderer::FrameNumber();
        if (frame == m_CollectedFrame) return;
        m_CollectedFrame = frame;

        u64              keep = Renderer::Device().FramesInFlight() + 2;
        Vector<upointer> stale;
        for (auto& [key, binding] : m_Textures)
            if (!binding.ImGuiOwned && binding.LastUsed + keep < frame)
                stale.PushBack(key);
        for (upointer key : stale) m_Textures.Erase(key);
    }

    void Backend::UpdateTexture(ImTextureData* texture)
    {
        auto& device = Renderer::Device();

        if (texture->Status == ImTextureStatus_WantCreate)
        {
            VtCoreAssertMsg(texture->Format == ImTextureFormat_RGBA32,
                            "ImGui: Only RGBA32 textures are supported");
            RHI::TextureDesc desc{};
            desc.Format    = RHI::Format::eRGBA8Unorm;
            desc.Extent    = {static_cast<u32>(texture->Width),
                              static_cast<u32>(texture->Height), 1};
            desc.Usage     = RHI::TextureUsage::eSampled | RHI::TextureUsage::eTransferDst;
            desc.DebugName = Prism::Format("ImGui texture {}", texture->UniqueID);
            // Created now (draw lists need the ID this frame), filled on the
            // render thread before anything draws with it.
            auto rhiTexture = device.CreateTexture(desc);

            ImTextureID id = TextureID(rhiTexture);
            m_Textures[static_cast<upointer>(id)].ImGuiOwned = true;
            texture->SetTexID(id);
            texture->SetStatus(ImTextureStatus_OK);

            Vector<u8> pixels(static_cast<usize>(texture->GetSizeInBytes()));
            std::memcpy(pixels.Raw(), texture->GetPixels(), pixels.Size());
            Renderer::Submit([rhiTexture, pixels](RenderContext& ctx)
                             { ctx.Device().UploadTexture(*rhiTexture, pixels.Raw(), pixels.Size()); });
            return;
        }

        if (texture->Status == ImTextureStatus_WantUpdates)
        {
            auto it = m_Textures.Find(static_cast<upointer>(texture->TexID));
            VtCoreAssertMsg(it != m_Textures.end(),
                            "ImGui: Update of a texture the backend never created");
            ::Ref<RHI::Texture> rhiTexture = it->Value.Texture;

            // Rows of a rect aren't contiguous in ImGui's pixels: pack them.
            for (const ImTextureRect& rect : texture->Updates)
            {
                usize      rowSize = static_cast<usize>(rect.w) * texture->BytesPerPixel;
                Vector<u8> packed(rowSize * rect.h);
                for (int y = 0; y < rect.h; ++y)
                    std::memcpy(packed.Raw() + rowSize * y,
                                texture->GetPixelsAt(rect.x, rect.y + y), rowSize);

                RHI::TextureRegion region{};
                region.X      = rect.x;
                region.Y      = rect.y;
                region.Extent = {rect.w, rect.h, 1};
                Renderer::Submit(
                    [rhiTexture, packed, region](RenderContext& ctx) {
                        ctx.Device().UploadTexture(*rhiTexture, packed.Raw(),
                                                   packed.Size(), region);
                    });
            }
            texture->SetStatus(ImTextureStatus_OK);
            return;
        }

        if (texture->Status == ImTextureStatus_WantDestroy
            && texture->UnusedFrames > static_cast<int>(device.FramesInFlight()))
        {
            m_Textures.Erase(static_cast<upointer>(texture->TexID));
            texture->SetTexID(ImTextureID_Invalid);
            texture->SetStatus(ImTextureStatus_Destroyed);
            texture->BackendUserData = nullptr;
        }
    }

    void Backend::ProcessTextures()
    {
        for (ImTextureData* texture : ImGui::GetPlatformIO().Textures)
            if (texture->Status != ImTextureStatus_OK) UpdateTexture(texture);
    }

    Backend::DrawSnapshot Backend::Capture(ImDrawData* drawData)
    {
        VtProfileFunction();
        DrawSnapshot snapshot{};
        if (!drawData) return snapshot;
        snapshot.DisplayPos       = drawData->DisplayPos;
        snapshot.DisplaySize      = drawData->DisplaySize;
        snapshot.FramebufferScale = drawData->FramebufferScale;
        snapshot.TotalVertices    = static_cast<usize>(drawData->TotalVtxCount);
        snapshot.TotalIndices     = static_cast<usize>(drawData->TotalIdxCount);

        for (const ImDrawList* list : drawData->CmdLists)
        {
            DrawList& copy = snapshot.Lists.EmplaceBack();
            copy.Vertices  = CopyOf(list->VtxBuffer);
            copy.Indices   = CopyOf(list->IdxBuffer);
            for (const ImDrawCmd& draw : list->CmdBuffer)
            {
                DrawCommand& command = copy.Commands.EmplaceBack();
                if (draw.UserCallback)
                {
                    // Callbacks run on the game thread's ImGui state; only the
                    // reset marker can cross to the render thread.
                    if (draw.UserCallback == ImDrawCallback_ResetRenderState)
                        command.ResetRenderState = true;
                    else
                        VtCoreWarnOnce("ImGui: Draw callbacks other than "
                                       "ResetRenderState are ignored");
                    continue;
                }
                command.ClipRect  = draw.ClipRect;
                command.ElemCount = draw.ElemCount;
                command.IdxOffset = draw.IdxOffset;
                command.VtxOffset = draw.VtxOffset;
                command.Group     = BindingFor(draw.GetTexID());
            }
        }
        return snapshot;
    }

    //------------------------------------------------------------------------
    // Rendering (render thread) ---------------------------------------------
    //------------------------------------------------------------------------

    ::Ref<RHI::Pipeline> Backend::PipelineFor(RHI::Format format)
    {
        RHI::VertexBinding vertex{};
        vertex.Stride = sizeof(ImDrawVert);
        vertex.Attributes.PushBack({0, RHI::Format::eRG32Float,
                                    static_cast<u32>(offsetof(ImDrawVert, pos))});
        vertex.Attributes.PushBack({1, RHI::Format::eRG32Float,
                                    static_cast<u32>(offsetof(ImDrawVert, uv))});
        vertex.Attributes.PushBack({2, RHI::Format::eRGBA8Unorm,
                                    static_cast<u32>(offsetof(ImDrawVert, col))});

        RHI::GraphicsPipelineDesc desc{};
        desc.Shader = m_Shader;
        desc.VertexBindings.PushBack(vertex);
        desc.Raster.Cull             = RHI::CullMode::eNone;
        desc.DepthStencil.DepthTest  = false;
        desc.DepthStencil.DepthWrite = false;
        desc.Blend.PushBack(RHI::BlendAttachment::AlphaBlend());
        desc.ColorFormats.PushBack(format);
        desc.DebugName = "ImGui";
        return Renderer::Pipelines().Get(desc);
    }

    void Backend::UploadGeometry(const DrawSnapshot& snapshot, FrameGeometry& geometry)
    {
        auto& device     = Renderer::Device();
        usize vertexSize = snapshot.TotalVertices * sizeof(ImDrawVert);
        usize indexSize  = snapshot.TotalIndices * sizeof(ImDrawIdx);

        // Grow geometrically; the old buffers live until the GPU is done.
        auto ensure = [&](::Ref<RHI::Buffer>& buffer, usize size,
                          RHI::BufferUsage usage, const char* name)
        {
            if (buffer && buffer->Size() >= size) return;
            usize capacity = buffer ? buffer->Size() : 64 * 1024;
            while (capacity < size) capacity *= 2;
            RHI::BufferDesc desc{};
            desc.Size      = capacity;
            desc.Usage     = usage;
            desc.Memory    = RHI::MemoryUsage::eCpuToGpu;
            desc.DebugName = name;
            buffer         = device.CreateBuffer(desc);
        };
        ensure(geometry.Vertices, vertexSize, RHI::BufferUsage::eVertex,
               "ImGui vertices");
        ensure(geometry.Indices, indexSize, RHI::BufferUsage::eIndex, "ImGui indices");

        usize vertexOffset = 0;
        usize indexOffset  = 0;
        for (const auto& list : snapshot.Lists)
        {
            usize vertices = list.Vertices.Size() * sizeof(ImDrawVert);
            usize indices  = list.Indices.Size() * sizeof(ImDrawIdx);
            if (vertices) geometry.Vertices->Write(list.Vertices.Raw(), vertices, vertexOffset);
            if (indices) geometry.Indices->Write(list.Indices.Raw(), indices, indexOffset);
            vertexOffset += vertices;
            indexOffset += indices;
        }
    }

    void Backend::Render(const DrawSnapshot& snapshot, RHI::CommandList& cmd,
                         RHI::Texture& target, bool clear, ViewportData& viewport)
    {
        VtProfileFunction();
        f32 width  = snapshot.DisplaySize.x * snapshot.FramebufferScale.x;
        f32 height = snapshot.DisplaySize.y * snapshot.FramebufferScale.y;
        if (width <= 0.0f || height <= 0.0f) return;

        RHI::ScopedLabel   label(cmd, "ImGui");
        RHI::ScopedGpuZone zone(cmd, "ImGui");
        auto& geometry = viewport.Frames[Renderer::Device().FrameIndex()];
        if (snapshot.TotalVertices > 0) UploadGeometry(snapshot, geometry);

        cmd.Transition(target, RHI::ResourceState::eColorAttachment);
        RHI::RenderingDesc    rendering{};
        RHI::ColorAttachment& color = rendering.Colors.EmplaceBack();
        color.Texture               = ::Ref<RHI::Texture>(&target);
        color.Load  = clear ? RHI::LoadOp::eClear : RHI::LoadOp::eLoad;
        color.Clear = {0.0f, 0.0f, 0.0f, 1.0f};
        cmd.BeginRendering(rendering);

        if (snapshot.TotalVertices > 0)
        {
            auto pipeline = PipelineFor(target.Format());
            auto setup    = [&]
            {
                cmd.BindPipeline(*pipeline);
                cmd.BindVertexBuffer(0, *geometry.Vertices);
                cmd.BindIndexBuffer(*geometry.Indices, sizeof(ImDrawIdx) == 2
                                                           ? RHI::IndexType::eU16
                                                           : RHI::IndexType::eU32);
                // ImGui's coordinates (DisplayPos..DisplayPos+Size) to NDC.
                PushConstants constants{};
                constants.Scale[0] = 2.0f / snapshot.DisplaySize.x;
                constants.Scale[1] = 2.0f / snapshot.DisplaySize.y;
                constants.Translate[0] = -1.0f - snapshot.DisplayPos.x * constants.Scale[0];
                constants.Translate[1] = -1.0f - snapshot.DisplayPos.y * constants.Scale[1];
                cmd.PushConstants(RHI::ShaderStage::eVertex, constants);
            };
            setup();

            ImVec2          clipOffset = snapshot.DisplayPos;
            ImVec2          clipScale  = snapshot.FramebufferScale;
            u32             vertexBase = 0;
            u32             indexBase  = 0;
            RHI::BindGroup* bound      = nullptr;

            for (const auto& list : snapshot.Lists)
            {
                for (const auto& draw : list.Commands)
                {
                    if (draw.ResetRenderState)
                    {
                        setup();
                        bound = nullptr;
                        continue;
                    }
                    if (!draw.Group || draw.ElemCount == 0) continue;

                    f32 minX = (draw.ClipRect.x - clipOffset.x) * clipScale.x;
                    f32 minY = (draw.ClipRect.y - clipOffset.y) * clipScale.y;
                    f32 maxX = (draw.ClipRect.z - clipOffset.x) * clipScale.x;
                    f32 maxY = (draw.ClipRect.w - clipOffset.y) * clipScale.y;
                    if (minX < 0.0f) minX = 0.0f;
                    if (minY < 0.0f) minY = 0.0f;
                    if (maxX > width) maxX = width;
                    if (maxY > height) maxY = height;
                    if (maxX <= minX || maxY <= minY) continue;

                    cmd.SetScissor({{static_cast<i32>(minX), static_cast<i32>(minY)},
                                    {static_cast<u32>(maxX - minX),
                                     static_cast<u32>(maxY - minY)}});
                    if (draw.Group.Raw() != bound)
                    {
                        cmd.BindGroup(0, *draw.Group);
                        bound = draw.Group.Raw();
                    }
                    cmd.DrawIndexed(draw.ElemCount, 1, draw.IdxOffset + indexBase,
                                    static_cast<i32>(draw.VtxOffset + vertexBase));
                }
                vertexBase += list.Vertices.Size();
                indexBase += list.Indices.Size();
            }
        }
        cmd.EndRendering();
    }

    //------------------------------------------------------------------------
    // Frame (game thread) ---------------------------------------------------
    //------------------------------------------------------------------------

    void Backend::RenderViewports(RenderTargetWindow& mainWindow)
    {
        VtProfileFunction();
        ProcessTextures();
        CollectUnusedTextures();

        ::Ref<RHI::SwapChain> swapChain = mainWindow.SwapChain();
        if (swapChain && !mainWindow.IsMinimized())
        {
            Renderer::Submit(
                [this, swapChain, snapshot = Capture(ImGui::GetDrawData())](
                    RenderContext& ctx)
                {
                    if (!swapChain->AcquireNextImage()) return;
                    Render(snapshot, ctx.Cmd(), *swapChain->CurrentImage(), true,
                           m_MainViewport);
                    ctx.Present(Span<const ::Ref<RHI::SwapChain>>(&swapChain, 1));
                });
        }

        if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        {
            ImGui::UpdatePlatformWindows();
            // Captures each platform window (RenderWindow) for the render
            // thread.
            ImGui::RenderPlatformWindowsDefault();
        }
    }

    //------------------------------------------------------------------------
    // Platform windows (game thread) ----------------------------------------
    //------------------------------------------------------------------------

    void Backend::CreateWindow(ImGuiViewport* viewport)
    {
        auto* platform = static_cast<ImGuiViewportData*>(viewport->PlatformUserData);
        VtCoreAssertMsg(platform && platform->Window,
                        "ImGui: Renderer window without a platform window");

        auto* data = new ViewportData;
        data->Frames.Resize(Renderer::Device().FramesInFlight());
        RHI::SwapChainDesc desc{};
        desc.Window = platform->Window;
        // Never FIFO: on Wayland a FIFO present can block until the
        // compositor shows the surface, which it may never do for a popup
        // that's hidden or not yet mapped, stalling the render thread (and
        // with it every window). The main window paces the frame anyway.
        desc.VSync      = false;
        data->SwapChain = Renderer::Device().CreateSwapChain(desc);
        viewport->RendererUserData = data;
    }

    void Backend::DestroyWindow(ImGuiViewport* viewport)
    {
        // The main viewport's swapchain belongs to its RenderTargetWindow.
        if (viewport == ImGui::GetMainViewport()) return;
        auto* data = static_cast<ViewportData*>(viewport->RendererUserData);
        if (!data) return;
        // The render thread may still present to it, and the swapchain must
        // go before the platform window (destroyed right after this).
        Renderer::Flush();
        delete data;
        viewport->RendererUserData = nullptr;
    }

    void Backend::SetWindowSize(ImGuiViewport* viewport, ImVec2)
    {
        auto* data = static_cast<ViewportData*>(viewport->RendererUserData);
        if (data && data->SwapChain) data->SwapChain->RequestResize();
    }

    void Backend::RenderWindow(ImGuiViewport* viewport, void*)
    {
        auto* data = static_cast<ViewportData*>(viewport->RendererUserData);
        if (!data || !data->SwapChain) return;
        // Not accepting content yet (Wayland popup awaiting its configure).
        auto* window = static_cast<Window*>(viewport->PlatformHandle);
        if (window && !window->IsReadyForRendering()) return;

        bool clear = !(viewport->Flags & ImGuiViewportFlags_NoRendererClear);
        Renderer::Submit(
            [data, clear, snapshot = Get().Capture(viewport->DrawData)](RenderContext& ctx)
            {
                if (!data->SwapChain->AcquireNextImage()) return;
                auto& device = ctx.Device();
                auto& cmd    = device.BeginCommandList();
                Get().Render(snapshot, cmd, *data->SwapChain->CurrentImage(), clear, *data);
                device.Submit(cmd, Span<const ::Ref<RHI::SwapChain>>(&data->SwapChain, 1));
            });
    }
} // namespace Vortex
