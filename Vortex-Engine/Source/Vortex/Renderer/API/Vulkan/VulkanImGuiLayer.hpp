/*
 * Created by v1tr10l7 on 28.07.2024.
 * Copyright (c) 2024-2024, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include <Prism/Containers/Array.hpp>
#include <Vortex/Core/Core.hpp>
#include <Vortex/Core/Layer.hpp>

#include <Vortex/Window/Window.hpp>

#include <imgui.h>
#undef CreateWindow

namespace Vortex
{
    struct ImGui_ImplVortex_Data
    {
        Vortex::Window* MainWindow        = nullptr;
        f64             Time              = 0.0;
        Vortex::Window* MouseWindow       = nullptr;
        ImVec2          LastValidMousePos = {};
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

    class VulkanImGuiLayer final : public Layer
    {
      public:
        VulkanImGuiLayer(StringView name)
            : Layer(name)
        {
        }
        ~VulkanImGuiLayer() = default;

        virtual void OnAttach() override;
        virtual void OnDetach() override;

        void         Begin();
        void         End();

        virtual void OnUpdate(Timestep deltaTime) override;
        virtual void OnRender() override;
        virtual void OnImGuiRender() override;

      private:
        vk::DescriptorPool        m_DescriptorPool = VK_NULL_HANDLE;
        vk::CommandPool           m_CommandPool    = VK_NULL_HANDLE;
        Vector<vk::CommandBuffer> m_CommandBuffers;

        static void               CreateCursors();
        static void               SetUpEvents();
        static void               InitializePlatformInterface();

        static void               UpdateMouseData();
        static void               UpdateMouseCursor();
        static void               UpdateMonitors();

        //--------------------------------------------------------------------
        // Events ------------------------------------------------------------
        //--------------------------------------------------------------------
        static bool   OnKeyPressed(Window* window, Input::KeyCode key,
                                   u32 repeatCount);
        static bool   OnKeyReleased(Window* window, Input::KeyCode key);
        static bool   OnKeyTyped(Window* window, u32 key);

        static bool   OnMouseButtonPressed(Window*          window,
                                           Input::MouseCode button);
        static bool   OnMouseButtonReleased(Window*          window,
                                            Input::MouseCode button);
        static bool   OnMouseScrolled(Window* window, f64 xoffset, f64 yoffset);
        static bool   OnMouseMoved(Window* window, f64 x, f64 y);
        static bool   OnMouseEntered(Window* window);
        static bool   OnMouseLeft(Window* window);

        static bool   OnWindowMoved(Window* window, i32 x, i32 y);
        static bool   OnWindowResized(Window* window, i32 width, i32 height);
        static bool   OnWindowFocusGained(Window* window);
        static bool   OnWindowFocusLost(Window* window);
        static bool   OnWindowClosed(Window* window);

        static bool   OnMonitorStateUpdate(Monitor*, MonitorState);

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

        static i32  CreateVulkanSurface(ImGuiViewport* viewport,
                                        ImU64          vkInstance,
                                        const void*    vkAllocator,
                                        ImU64*         outVkSurface);
    };
}; // namespace Vortex
