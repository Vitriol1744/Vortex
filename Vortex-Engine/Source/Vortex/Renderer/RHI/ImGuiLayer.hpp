/*
 * Created by v1tr10l7 on 10.07.2026.
 * Copyright (c) 2024-2026, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include <Prism/Containers/Array.hpp>
#include <Vortex/Core/Layer.hpp>
#include <Vortex/Renderer/RHI/Export.hpp>
#include <Vortex/Renderer/RenderTargetWindow.hpp>

#include <imgui.h>
#undef CreateWindow

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

    class VT_RENDERER_RHI_API ImGuiLayer : public Layer
    {
      public:
        ImGuiLayer(StringView name, RenderTargetWindow& mainWindow,
                   EventBus& eventBus)
            : Layer(name)
            , m_MainWindow(mainWindow)
            , m_EventBus(eventBus)
        {
        }
        ~ImGuiLayer() = default;

        virtual void OnAttach() override final;
        virtual void OnDetach() override final;

        void         Begin();
        void         End();

        virtual void OnUpdate(Timestep deltaTime) override;
        virtual void OnRender() override;
        virtual void OnImGuiRender() override;

      protected:
        RenderTargetWindow& m_MainWindow;

        virtual void        OnInitializeRendererInterface() = 0;
        virtual void        OnShutdownRendererInterface()   = 0;

        virtual void        OnNewFrame()                    = 0;
        virtual void        OnRenderDrawData()              = 0;

      private:
        EventBus&   m_EventBus;

        void        CreateCursors();
        void        SetUpEvents();
        void        InitializePlatformInterface();

        static void UpdateMouseData();
        static void UpdateMouseCursor();
        void        UpdateMonitors();

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
