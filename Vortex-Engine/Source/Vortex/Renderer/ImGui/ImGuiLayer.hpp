/*
 * Created by v1tr10l7 on 17.07.2024.
 * Copyright (c) 2024-2026, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include <Vortex/Core/Layer.hpp>
#include <Vortex/Renderer/Export.hpp>
#include <Vortex/Renderer/RenderTargetWindow.hpp>

#include <imgui.h>
#undef CreateWindow

import Prism.Containers.Array;
import Prism.Containers.UnorderedMap;
import Prism.Containers.Vector;
import Prism.Memory.Ref;
import Vortex.RHI;

namespace Vortex
{
    struct ImGui_ImplVortex_Data
    {
        Vortex::RenderTargetWindow* MainWindow        = nullptr;
        f64                         Time              = 0.0;
        Vortex::Window*             MouseWindow       = nullptr;
        ImVec2                      LastValidMousePos = {};
        Array<Vortex::Window*, ToUnderlying(Vortex::Input::KeyCode::eKeyCount)>
             KeyOwnerWindows    = {};
        bool WantUpdateMonitors = false;
    };

    struct ImGuiViewportData
    {
        Vortex::Window* Window                     = nullptr;
        bool            WindowOwned                = false;
        int             IgnoreWindowPosEventFrame  = -1;
        int             IgnoreWindowSizeEventFrame = -1;
    };

    /**
     * Dear ImGui for the engine: the platform side (windows, input, monitors,
     * multi-viewports) is implemented on Vortex's Window API, the renderer
     * side (VortexBackend) on the RHI, so it runs on any RHI backend.
     *
     * Per frame:
     *
     *   imgui.Begin();
     *   ... ImGui calls ...
     *   imgui.End();     // renders into the main window and every platform
     *                    // window, inside the current Renderer frame
     */
    class VT_RENDERER_API ImGuiLayer final : public Layer
    {
      public:
        /**
         * ImGui's renderer backend on the RHI. Handles ImGui 1.92 dynamic
         * textures (font atlas included) and user textures: an ImTextureID
         * is an RHI::Texture* (see UI::TextureID), resolved to a bind group
         * cached here.
         */
        class VT_RENDERER_API VortexBackend
        {
          public:
            VortexBackend();
            ~VortexBackend();
            VortexBackend(const VortexBackend&)            = delete;
            VortexBackend& operator=(const VortexBackend&) = delete;

            static VortexBackend& Get();

            /// Game thread, after ImGui::Render(): captures the main viewport
            /// and every platform window and queues their rendering (and any
            /// texture uploads) on the render thread.
            void        RenderViewports(RenderTargetWindow& mainWindow);

            /// ImTextureID for an RHI texture (kept alive while ImGui uses
            /// it). Game thread.
            ImTextureID TextureID(const ::Ref<RHI::Texture>& texture);

          private:
            struct TextureBinding
            {
                ::Ref<RHI::Texture>   Texture;
                ::Ref<RHI::BindGroup> Group;
                /// Game frame of the last draw (or registration) using it.
                u64                   LastUsed   = 0;
                /// Owned by an ImTextureData (font atlas...), not a user.
                bool                  ImGuiOwned = false;
            };

            // A copy of ImDrawData the render thread can use while the game
            // thread builds the next ImGui frame.
            struct DrawCommand
            {
                ImVec4                ClipRect{};
                u32                   ElemCount = 0;
                u32                   IdxOffset = 0;
                u32                   VtxOffset = 0;
                ::Ref<RHI::BindGroup> Group;
                bool                  ResetRenderState = false;
            };
            struct DrawList
            {
                Vector<ImDrawVert>  Vertices;
                Vector<ImDrawIdx>   Indices;
                Vector<DrawCommand> Commands;
            };
            struct DrawSnapshot
            {
                ImVec2           DisplayPos{};
                ImVec2           DisplaySize{};
                ImVec2           FramebufferScale{};
                usize            TotalVertices = 0;
                usize            TotalIndices  = 0;
                Vector<DrawList> Lists;
            };

            /// Geometry of one viewport, one set per frame in flight.
            struct FrameGeometry
            {
                ::Ref<RHI::Buffer> Vertices;
                ::Ref<RHI::Buffer> Indices;
            };
            struct ViewportData
            {
                ::Ref<RHI::SwapChain> SwapChain;
                /// Render thread only.
                Vector<FrameGeometry> Frames;
            };

            ::Ref<RHI::Shader>                      m_Shader;
            /// Game thread only.
            UnorderedMap<upointer, TextureBinding>  m_Textures;
            ViewportData                            m_MainViewport;
            u64                                     m_CollectedFrame = 0;

            // Game thread
            DrawSnapshot    Capture(ImDrawData* drawData);
            ::Ref<RHI::BindGroup> BindingFor(ImTextureID id);
            void            ProcessTextures();
            void            UpdateTexture(ImTextureData* texture);
            void            CollectUnusedTextures();

            // Render thread
            void            Render(const DrawSnapshot& snapshot, RHI::CommandList& cmd,
                                   RHI::Texture& target, bool clear,
                                   ViewportData& viewport);
            ::Ref<RHI::Pipeline> PipelineFor(RHI::Format format);
            void            UploadGeometry(const DrawSnapshot& snapshot,
                                           FrameGeometry&      geometry);

            static void     CreateWindow(ImGuiViewport* viewport);
            static void     DestroyWindow(ImGuiViewport* viewport);
            static void     SetWindowSize(ImGuiViewport* viewport, ImVec2 size);
            static void     RenderWindow(ImGuiViewport* viewport, void* arg);
        };

        ImGuiLayer(StringView name, RenderTargetWindow& mainWindow,
                   EventBus& eventBus)
            : Layer(name)
            , m_MainWindow(mainWindow)
            , m_EventBus(eventBus)
        {
        }
        ~ImGuiLayer() = default;

        virtual void OnAttach() override;
        virtual void OnDetach() override;

        void         Begin();
        /// Finalizes the ImGui frame and renders it (inside a Renderer frame).
        void         End();

        virtual void OnUpdate(Timestep deltaTime) override;
        virtual void OnRender() override;
        virtual void OnImGuiRender() override;

      private:
        RenderTargetWindow&  m_MainWindow;
        EventBus&            m_EventBus;
        Scope<VortexBackend> m_Backend;

        void                 CreateCursors();
        void                 SetUpEvents();
        void                 InitializePlatformInterface();
        static void          UpdateMouseData();
        static void          UpdateMouseCursor();
        void                 UpdateMonitors();

        //--------------------------------------------------------------------
        // Events ------------------------------------------------------------
        //--------------------------------------------------------------------

        static void OnKeyPressed(const KeyPressedEvent& event);
        static void OnKeyReleased(const KeyReleasedEvent& event);
        static void OnKeyTyped(const KeyTypedEvent& event);
        static void OnMouseButtonPressed(const MouseButtonPressedEvent& event);
        static void
        OnMouseButtonReleased(const MouseButtonReleasedEvent& event);
        static void   OnMouseScrolled(const MouseScrolledEvent& event);
        static void   OnMouseMoved(const MouseMovedEvent& event);
        static void   OnMouseEntered(const MouseEnteredEvent& event);
        static void   OnMouseLeft(const MouseLeftEvent& event);
        static void   OnWindowMoved(const WindowMovedEvent& e);
        static void   OnWindowResized(const WindowResizedEvent& event);
        static void   OnWindowFocusGained(const WindowFocusedEvent& e);
        static void   OnWindowFocusLost(const WindowFocusLostEvent& e);
        static void   OnWindowClosed(const WindowClosedEvent& e);
        static void   OnMonitorStateUpdate(const MonitorStateChangedEvent&);

        //--------------------------------------------------------------------
        // Window helpers ----------------------------------------------------
        //--------------------------------------------------------------------

        static void   CreateWindow(ImGuiViewport* viewport);
        static void   DestroyWindow(ImGuiViewport* viewport);
        static void   ShowWindow(ImGuiViewport* viewport);
        static ImVec2 WindowPosition(ImGuiViewport* viewport);
        static void SetWindowPosition(ImGuiViewport* viewport, ImVec2 position);
        static ImVec2 WindowSize(ImGuiViewport* viewport);
        static void   SetWindowSize(ImGuiViewport* viewport, ImVec2 size);
        static void SetWindowTitle(ImGuiViewport* viewport, const char* title);
        static void RequestWindowFocus(ImGuiViewport* viewport);
        static bool IsWindowFocused(ImGuiViewport* viewport);
        static bool IsWindowMinimized(ImGuiViewport* viewport);
        static void SetWindowOpacity(ImGuiViewport* viewport, f32 opacity);
    };
}; // namespace Vortex
