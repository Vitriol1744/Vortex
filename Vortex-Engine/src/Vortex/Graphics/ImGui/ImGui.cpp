//
// Created by Vitriol1744 on 18.08.2021.
//
#include <Core/Time.hpp>
#include "ImGui.hpp"

#include "Vortex/Core/Input/Input.hpp"

#include "imgui/imgui.h"
#include "Vortex/Graphics/ImGui/Backend_ImGui_OpenGL3.hpp"
#include "Vortex/Graphics/Window/WindowManager.hpp"

namespace
{
    using namespace Vortex;
    using namespace Graphics;
    struct ImGui_ImplVortex_Data
    {
        Ref<IWindow> window;
        float time;
        bool wantUpdateMonitors;
    };
    struct ImGui_ImplVortex_ViewportData
    {
        Ref<IWindow> window;
        bool         windowOwned;
        int          ignoreWindowPosEventFrame;
        int          ignoreWindowSizeEventFrame;

        ImGui_ImplVortex_ViewportData()  { window = nullptr; windowOwned = false; ignoreWindowSizeEventFrame = ignoreWindowPosEventFrame = -1; }
    };

    void CreateWindow(ImGuiViewport* viewport)
    {
        ImGui_ImplVortex_Data* bd = (ImGui_ImplVortex_Data*)ImGui::GetIO().BackendPlatformUserData;
        ImGui_ImplVortex_ViewportData* vd = IM_NEW(ImGui_ImplVortex_ViewportData)();
        viewport->PlatformUserData = vd;

        Ref<IWindow> sharedWindow = bd->window; // if opengl
        vd->window = IWindow::CreateWindow((int)viewport->Size.x, (int)viewport->Size.y, 32, "No Title Yet", sharedWindow);
        vd->windowOwned = true;
        viewport->PlatformHandle = (void*)vd->window.get();
        vd->window->SetPosition((int)viewport->Pos.x, (int)viewport->Pos.y);
        vd->window->SetVisible(false);
        vd->window->SetStyle(WindowStyle::eNone);

        VTAddListener(WindowEvents::keyPressedEvent, ImGuiRenderer::OnKeyPressedEvent, VT_PH(1), VT_PH(2), VT_PH(3));
        VTAddListener(WindowEvents::keyReleasedEvent, ImGuiRenderer::OnKeyReleasedEvent, VT_PH(1), VT_PH(2));
        VTAddListener(WindowEvents::keyTypedEvent, ImGuiRenderer::OnKeyTypedEvent, VT_PH(1), VT_PH(2));
        VTAddListener(WindowEvents::mouseButtonPressedEvent, ImGuiRenderer::OnMouseButtonPressedEvent, VT_PH(1), VT_PH(2), VT_PH(3));
        VTAddListener(WindowEvents::mouseButtonReleasedEvent, ImGuiRenderer::OnMouseButtonReleasedEvent, VT_PH(1), VT_PH(2));
        VTAddListener(WindowEvents::mouseScrolledEvent, ImGuiRenderer::OnMouseScrolledEvent, VT_PH(1), VT_PH(2));
        VTAddListener(WindowEvents::mouseMovedEvent, ImGuiRenderer::OnMouseMovedEvent, VT_PH(1), VT_PH(2));
        VTAddListener(WindowEvents::windowResizedEvent, ImGuiRenderer::OnWindowResizedEvent, VT_PH(1), VT_PH(2));
        VTAddListener(WindowEvents::windowClosedEvent, ImGuiRenderer::OnWindowClosedEvent, VT_PH(1));
        VTAddListener(WindowEvents::windowMovedEvent, ImGuiRenderer::OnWindowMovedEvent, VT_PH(1), VT_PH(2));
    }
    void DestroyWindow(ImGuiViewport* viewport)
    {
        if (ImGui_ImplVortex_ViewportData* vd = (ImGui_ImplVortex_ViewportData*)viewport->PlatformUserData)
        {
            IM_DELETE(vd);
        }
        viewport->PlatformUserData = viewport->PlatformHandle = nullptr;
    }
    void ShowWindow(ImGuiViewport* viewport)
    {
        ((ImGui_ImplVortex_ViewportData*)(viewport->PlatformUserData))->window->SetVisible(true);
    }
    void SetWindowPosition(ImGuiViewport* viewport, ImVec2 position)
    {
        ImGui_ImplVortex_ViewportData* vd = (ImGui_ImplVortex_ViewportData*)viewport->PlatformUserData;
        vd->ignoreWindowPosEventFrame = ImGui::GetFrameCount();
        vd->window->SetPosition(static_cast<uint32>(position.x), static_cast<uint32>(position.y));
    }
    ImVec2 GetWindowPosition(ImGuiViewport* viewport)
    {
        Math::Vec2 pos = ((ImGui_ImplVortex_ViewportData*)(viewport->PlatformUserData))->window->GetPosition();
        return ImVec2(pos.x, pos.y);
    }
    void SetWindowSize(ImGuiViewport* viewport, ImVec2 size)
    {
        ImGui_ImplVortex_ViewportData* vd = (ImGui_ImplVortex_ViewportData*)viewport->PlatformUserData;
        vd->ignoreWindowSizeEventFrame = ImGui::GetFrameCount();
        vd->window->SetSize(static_cast<uint32>(size.x), static_cast<uint32>(size.y));
    }
    ImVec2 GetWindowSize(ImGuiViewport* viewport)
    {
        Math::Vec2 size = ((ImGui_ImplVortex_ViewportData*)(viewport->PlatformUserData))->window->GetSize();
        return ImVec2(size.x, size.y);
    }
    void SetWindowFocus(ImGuiViewport* viewport)
    {
        ((ImGui_ImplVortex_ViewportData*)(viewport->PlatformUserData))->window->RequestFocus();
    }
    bool GetWindowFocus(ImGuiViewport* viewport)
    {
        return ((ImGui_ImplVortex_ViewportData*)(viewport->PlatformUserData))->window->IsFocused();
    }
    bool GetWindowMinimized(ImGuiViewport* viewport)
    {
        return false;
    }
    void SetWindowTitle(ImGuiViewport* viewport, const char* title)
    {
        ((ImGui_ImplVortex_ViewportData*)(viewport->PlatformUserData))->window->SetTitle(title);
    }
    void RenderWindow(ImGuiViewport* viewport, void*)
    {
        ((ImGui_ImplVortex_ViewportData*)(viewport->PlatformUserData))->window->ActivateContext();
    }
    void SwapBuffers(ImGuiViewport* viewport, void*)
    {
        ((ImGui_ImplVortex_ViewportData*)(viewport->PlatformUserData))->window->Present();
    }
}

namespace Vortex::Graphics
{
    using namespace Input;
    Ref<IWindow> ImGuiRenderer::window = nullptr;

    void ImGuiRenderer::Initialize(Ref<IWindow> window)
    {
        ImGuiRenderer::window = window;
        
        ::ImGui::CreateContext();
        ::ImGui::StyleColorsDark();

        ImGuiIO& io = ::ImGui::GetIO();
        ImGui_ImplVortex_Data* bd = IM_NEW(ImGui_ImplVortex_Data)();
        io.BackendPlatformUserData = (void*)bd;
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;       // Enable Keyboard Controls
        //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking
        io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;         // Enable Multi-Viewport / Platform Windows
        //io.ConfigFlags |= ImGuiConfigFlags_ViewportsNoTaskBarIcons;
        //io.ConfigFlags |= ImGuiConfigFlags_ViewportsNoMerge;

        io.BackendFlags |= ImGuiBackendFlags_HasMouseCursors;
        io.BackendFlags |= ImGuiBackendFlags_HasSetMousePos;
        io.BackendFlags |= ImGuiBackendFlags_PlatformHasViewports;
        io.BackendFlags |= ImGuiBackendFlags_HasMouseHoveredViewport; // We can set io.MouseHoveredViewport correctly (optional, not easy)

        bd->window = window;
        bd->time = 0.0;
        bd->wantUpdateMonitors = true;

        io.BackendFlags |= ImGuiBackendFlags_HasMouseCursors;
        io.BackendFlags |= ImGuiBackendFlags_HasSetMousePos;
        io.BackendFlags |= ImGuiBackendFlags_PlatformHasViewports;

        io.KeyMap[ImGuiKey_Tab          ] = static_cast<int>(KeyCode::eTab);
        io.KeyMap[ImGuiKey_LeftArrow    ] = static_cast<int>(KeyCode::eLeft);
        io.KeyMap[ImGuiKey_RightArrow   ] = static_cast<int>(KeyCode::eRight);
        io.KeyMap[ImGuiKey_UpArrow      ] = static_cast<int>(KeyCode::eUp);
        io.KeyMap[ImGuiKey_DownArrow    ] = static_cast<int>(KeyCode::eDown);
        io.KeyMap[ImGuiKey_PageUp       ] = static_cast<int>(KeyCode::ePageUp);
        io.KeyMap[ImGuiKey_PageDown     ] = static_cast<int>(KeyCode::ePageDown);
        io.KeyMap[ImGuiKey_Home         ] = static_cast<int>(KeyCode::eHome);
        io.KeyMap[ImGuiKey_End          ] = static_cast<int>(KeyCode::eEnd);
        io.KeyMap[ImGuiKey_Insert       ] = static_cast<int>(KeyCode::eInsert);
        io.KeyMap[ImGuiKey_Delete       ] = static_cast<int>(KeyCode::eDelete);
        io.KeyMap[ImGuiKey_Backspace    ] = static_cast<int>(KeyCode::eBackSpace);
        io.KeyMap[ImGuiKey_Space        ] = static_cast<int>(KeyCode::eSpace);
        io.KeyMap[ImGuiKey_Enter        ] = static_cast<int>(KeyCode::eEnter);
        io.KeyMap[ImGuiKey_Escape       ] = static_cast<int>(KeyCode::eEscape);
        io.KeyMap[ImGuiKey_KeyPadEnter  ] = static_cast<int>(KeyCode::eReturn);
        io.KeyMap[ImGuiKey_A            ] = static_cast<int>(KeyCode::eA);
        io.KeyMap[ImGuiKey_C            ] = static_cast<int>(KeyCode::eC);
        io.KeyMap[ImGuiKey_V            ] = static_cast<int>(KeyCode::eV);
        io.KeyMap[ImGuiKey_X            ] = static_cast<int>(KeyCode::eX);
        io.KeyMap[ImGuiKey_Y            ] = static_cast<int>(KeyCode::eY);
        io.KeyMap[ImGuiKey_Z            ] = static_cast<int>(KeyCode::eZ);

        //TODO: Clipboard Data
        //io.SetClipboardTextFn = ;
        //io.GetClipboardTextFn = ;
        //io.ClipboardUserData = ;

        ImGuiPlatformIO& platformIO = ImGui::GetPlatformIO();
        platformIO.Platform_CreateWindow = CreateWindow;
        platformIO.Platform_DestroyWindow = DestroyWindow;
        platformIO.Platform_ShowWindow = ShowWindow;
        platformIO.Platform_SetWindowPos = SetWindowPosition;
        platformIO.Platform_GetWindowPos = GetWindowPosition;
        platformIO.Platform_SetWindowSize = SetWindowSize;
        platformIO.Platform_GetWindowSize = GetWindowSize;
        platformIO.Platform_SetWindowFocus = SetWindowFocus;
        platformIO.Platform_GetWindowFocus = GetWindowFocus;
        platformIO.Platform_GetWindowMinimized = GetWindowMinimized;
        platformIO.Platform_SetWindowTitle = SetWindowTitle;
        platformIO.Platform_RenderWindow = RenderWindow;
        platformIO.Platform_SwapBuffers = SwapBuffers;

        ImGuiViewport* mainViewport = ::ImGui::GetMainViewport();
        mainViewport->PlatformHandle = (void*)window.get();

        ImGui_ImplVortex_ViewportData* vd = IM_NEW(ImGui_ImplVortex_ViewportData)();
        vd->window = bd->window;
        vd->windowOwned = true;
        mainViewport->PlatformUserData = vd;

        ImGuiPlatformMonitor monitor;
        monitor.MainPos = monitor.WorkPos = ImVec2((float)0, (float)0);
        monitor.MainSize = monitor.WorkSize = ImVec2(1920, 1080);
        platformIO.Monitors.push_back(monitor);

        ImGui_ImplOpenGL3_Init("#version 460");
        VTAddListener(WindowEvents::keyPressedEvent, ImGuiRenderer::OnKeyPressedEvent, VT_PH(1), VT_PH(2), VT_PH(3));
        VTAddListener(WindowEvents::keyReleasedEvent, ImGuiRenderer::OnKeyReleasedEvent, VT_PH(1), VT_PH(2));
        VTAddListener(WindowEvents::keyTypedEvent, ImGuiRenderer::OnKeyTypedEvent, VT_PH(1), VT_PH(2));
        VTAddListener(WindowEvents::mouseButtonPressedEvent, ImGuiRenderer::OnMouseButtonPressedEvent, VT_PH(1), VT_PH(2), VT_PH(3));
        VTAddListener(WindowEvents::mouseButtonReleasedEvent, ImGuiRenderer::OnMouseButtonReleasedEvent, VT_PH(1), VT_PH(2));
        VTAddListener(WindowEvents::mouseScrolledEvent, ImGuiRenderer::OnMouseScrolledEvent, VT_PH(1), VT_PH(2));
        VTAddListener(WindowEvents::mouseMovedEvent, ImGuiRenderer::OnMouseMovedEvent, VT_PH(1), VT_PH(2));
        VTAddListener(WindowEvents::windowResizedEvent, ImGuiRenderer::OnWindowResizedEvent, VT_PH(1), VT_PH(2));
    }
    void ImGuiRenderer::Shutdown()
    {
        
    }

    void ImGuiRenderer::Begin()
    {
        ImGuiIO& io = ImGui::GetIO();
        ImGuiPlatformIO& platformIO = ImGui::GetPlatformIO();

        float32 width  = static_cast<float32>(ImGuiRenderer::window->GetWidth());
        float32 height = static_cast<float32>(ImGuiRenderer::window->GetHeight());

        io.DisplaySize = ImVec2(width, height);
        io.DeltaTime = (float32)Time::DeltaTime();

        io.MouseHoveredViewport = 0;
        for (int n = 0; n < platformIO.Viewports.Size; n++)
        {
            ImGuiViewport* viewport = platformIO.Viewports[n];
            IWindow* window = (IWindow*)viewport->PlatformHandle;

            if (window->IsHovered() && !(viewport->Flags & ImGuiViewportFlags_NoInputs))
            {
                io.MouseHoveredViewport = viewport->ID;
            }
        }
        ImGui_ImplOpenGL3_NewFrame();
        ImGui::NewFrame();
    }
    void ImGuiRenderer::End()
    {
        ImGuiIO& io = ImGui::GetIO();
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        {
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
        }
    }

    bool ImGuiRenderer::OnKeyPressedEvent(IWindow*, Input::KeyCode keycode, uint32 repeatCount)
    {
        ImGuiIO& io = ::ImGui::GetIO();
        io.MouseDown[static_cast<uint32>(keycode)] = true;

        return false;
    }
    bool ImGuiRenderer::OnKeyReleasedEvent(IWindow*, Input::KeyCode keycode)
    {
        ImGuiIO& io = ::ImGui::GetIO();
        io.MouseDown[static_cast<uint32>(keycode)] = false;

        return false;
    }
    bool ImGuiRenderer::OnKeyTypedEvent(IWindow*, uint32 character)
    {
        return false;
    }
    bool ImGuiRenderer::OnMouseButtonPressedEvent(IWindow*, Input::MouseCode mousecode, bool doubleClick)
    {
        ImGuiIO& io = ::ImGui::GetIO();
        uint8 button = 0;

        switch (mousecode)
        {
            case Input::MouseCode::eLeft:
                button = 0;
                break;
            case Input::MouseCode::eMiddle:
                button = 2;
                break;
            case Input::MouseCode::eRight:
                button = 1;
                break;
            case MouseCode::eX1:
                button = 3;
                break;
            case MouseCode::eX2:
                button = 4;
                break;

            default:
                break;
        }
        io.MouseDown[button] = true;

        return false;
    }
    bool ImGuiRenderer::OnMouseButtonReleasedEvent(IWindow*, Input::MouseCode mousecode)
    {
        ImGuiIO& io = ::ImGui::GetIO();
        uint8 button = 0;

        switch (mousecode)
        {
            case Input::MouseCode::eLeft:
                button = 0;
                break;
            case Input::MouseCode::eMiddle:
                button = 2;
                break;
            case Input::MouseCode::eRight:
                button = 1;
                break;
            case Input::MouseCode::eX1:
                button = 3;
                break;
            case Input::MouseCode::eX2:
                button = 4;
                break;

            default:
                break;
        }
        io.MouseDown[button] = false;

        return false;
    }
    bool ImGuiRenderer::OnMouseScrolledEvent(IWindow*, Math::Vec2 wheel)
    {
        return false;
    }
    bool ImGuiRenderer::OnMouseMovedEvent(IWindow* window, Math::Vec2 position)
    {
        ImGuiIO& io = ::ImGui::GetIO();
        io.MousePos = ImVec2(position.x + window->GetPosition().x, position.y + window->GetPosition().y);

        return false;
    }
    bool ImGuiRenderer::OnWindowClosedEvent(IWindow* window)
    {
        if (ImGuiViewport* viewport = ImGui::FindViewportByPlatformHandle(window)) viewport->PlatformRequestClose = true;

        return false;
    }
    bool ImGuiRenderer::OnWindowResizedEvent(IWindow* window, Math::Vec2u size)
    {
        if (ImGuiViewport* viewport = ImGui::FindViewportByPlatformHandle(window))
        {
            if (ImGui_ImplVortex_ViewportData* vd = (ImGui_ImplVortex_ViewportData*)viewport->PlatformUserData)
            {
                bool ignore_event = (ImGui::GetFrameCount() <= vd->ignoreWindowSizeEventFrame + 1);
                if (ignore_event) return false;
            }
            viewport->PlatformRequestResize = true;
        }

        return false;
    }
    bool ImGuiRenderer::OnWindowMovedEvent(IWindow* window, Math::Vec2 position)
    {
        if (ImGuiViewport* viewport = ImGui::FindViewportByPlatformHandle(window))
        {
            if (ImGui_ImplVortex_ViewportData* vd = (ImGui_ImplVortex_ViewportData*)viewport->PlatformUserData)
            {
                bool ignore_event = (ImGui::GetFrameCount() <= vd->ignoreWindowPosEventFrame + 1);
                if (ignore_event) return false;
            }
            viewport->PlatformRequestMove = true;
        }

        return false;
    }
}