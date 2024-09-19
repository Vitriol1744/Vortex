/*
 * Created by v1tr10l7 on 28.07.2024.
 * Copyright (c) 2024-2024, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#include "vtpch.hpp"

#include "Vortex/Engine/Application.hpp"

#include "Vortex/Core/Time.hpp"
#include "Vortex/Renderer/API/Vulkan/VulkanImGuiLayer.hpp"
#include "Vortex/Renderer/API/Vulkan/VulkanRenderer.hpp"
#include "Vortex/Renderer/API/Vulkan/VulkanSurface.hpp"
#include "Vortex/Renderer/API/Vulkan/imgui_impl_vulkan.h"
#include <vulkan/vulkan_core.h>

#include <ImGuizmo.h>

namespace Vortex
{
    using Input::KeyCode;
    using Input::MouseCode;

    inline void SetupImGuiStyle(bool bStyleDark_, float alpha_)
    {
        ImGuiStyle& style           = ImGui::GetStyle();

        // light style from Pacôme Danhiez (user itamago)
        // https://github.com/ocornut/imgui/pull/511#issuecomment-175719267
        style.Alpha                 = 1.0f;
        style.FrameRounding         = 3.0f;
        style.Colors[ImGuiCol_Text] = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
        style.Colors[ImGuiCol_TextDisabled]
            = ImVec4(0.60f, 0.60f, 0.60f, 1.00f);
        style.Colors[ImGuiCol_WindowBg] = ImVec4(0.94f, 0.94f, 0.94f, 0.94f);
        style.Colors[ImGuiCol_DockingEmptyBg]
            = ImVec4(0.94f, 0.94f, 0.94f, 0.94f);
        style.Colors[ImGuiCol_ChildBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
        style.Colors[ImGuiCol_PopupBg] = ImVec4(1.00f, 1.00f, 1.00f, 0.94f);
        style.Colors[ImGuiCol_Border]  = ImVec4(0.00f, 0.00f, 0.00f, 0.39f);
        style.Colors[ImGuiCol_BorderShadow]
            = ImVec4(1.00f, 1.00f, 1.00f, 0.10f);
        style.Colors[ImGuiCol_FrameBg] = ImVec4(1.00f, 1.00f, 1.00f, 0.94f);
        style.Colors[ImGuiCol_FrameBgHovered]
            = ImVec4(0.26f, 0.59f, 0.98f, 0.40f);
        style.Colors[ImGuiCol_FrameBgActive]
            = ImVec4(0.26f, 0.59f, 0.98f, 0.67f);
        style.Colors[ImGuiCol_TitleBg] = ImVec4(0.96f, 0.96f, 0.96f, 1.00f);
        style.Colors[ImGuiCol_TitleBgCollapsed]
            = ImVec4(1.00f, 1.00f, 1.00f, 0.51f);
        style.Colors[ImGuiCol_TitleBgActive]
            = ImVec4(0.82f, 0.82f, 0.82f, 1.00f);
        style.Colors[ImGuiCol_MenuBarBg]   = ImVec4(0.86f, 0.86f, 0.86f, 1.00f);
        style.Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.98f, 0.98f, 0.98f, 0.53f);
        style.Colors[ImGuiCol_ScrollbarGrab]
            = ImVec4(0.69f, 0.69f, 0.69f, 1.00f);
        style.Colors[ImGuiCol_ScrollbarGrabHovered]
            = ImVec4(0.59f, 0.59f, 0.59f, 1.00f);
        style.Colors[ImGuiCol_ScrollbarGrabActive]
            = ImVec4(0.49f, 0.49f, 0.49f, 1.00f);
        // style.Colors[ImGuiCol_ComboBg]    = ImVec4(0.86f, 0.86f, 0.86f,
        // 0.99f);
        style.Colors[ImGuiCol_CheckMark]  = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
        style.Colors[ImGuiCol_SliderGrab] = ImVec4(0.24f, 0.52f, 0.88f, 1.00f);
        style.Colors[ImGuiCol_SliderGrabActive]
            = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
        style.Colors[ImGuiCol_Button] = ImVec4(0.26f, 0.59f, 0.98f, 0.40f);
        style.Colors[ImGuiCol_ButtonHovered]
            = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
        style.Colors[ImGuiCol_ButtonActive]
            = ImVec4(0.06f, 0.53f, 0.98f, 1.00f);
        style.Colors[ImGuiCol_Header] = ImVec4(0.26f, 0.59f, 0.98f, 0.31f);
        style.Colors[ImGuiCol_HeaderHovered]
            = ImVec4(0.26f, 0.59f, 0.98f, 0.80f);
        style.Colors[ImGuiCol_HeaderActive]
            = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
        // style.Colors[ImGuiCol_Col] = ImVec4(0.39f, 0.39f, 0.39f, 1.00f);
        // style.Colors[ImGuiCol_ColumnHovered]
        //     = ImVec4(0.26f, 0.59f, 0.98f, 0.78f);
        // style.Colors[ImGuiCol_ColumnActive]
        //     = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
        style.Colors[ImGuiCol_ResizeGrip] = ImVec4(1.00f, 1.00f, 1.00f, 0.50f);
        style.Colors[ImGuiCol_ResizeGripHovered]
            = ImVec4(0.26f, 0.59f, 0.98f, 0.67f);
        style.Colors[ImGuiCol_ResizeGripActive]
            = ImVec4(0.26f, 0.59f, 0.98f, 0.95f);
        // style.Colors[ImGuiCol_CloseButton] = ImVec4(0.59f, 0.59f, 0.59f,
        // 0.50f); style.Colors[ImGuiCol_CloseButtonHovered]
        //     = ImVec4(0.98f, 0.39f, 0.36f, 1.00f);
        // style.Colors[ImGuiCol_CloseButtonActive]
        //     = ImVec4(0.98f, 0.39f, 0.36f, 1.00f);
        style.Colors[ImGuiCol_PlotLines] = ImVec4(0.39f, 0.39f, 0.39f, 1.00f);
        style.Colors[ImGuiCol_PlotLinesHovered]
            = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
        style.Colors[ImGuiCol_PlotHistogram]
            = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
        style.Colors[ImGuiCol_PlotHistogramHovered]
            = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
        style.Colors[ImGuiCol_TextSelectedBg]
            = ImVec4(0.26f, 0.59f, 0.98f, 0.35f);
        // style.Colors[ImGuiCol_ModalWindowDarkening]
        //     = ImVec4(0.20f, 0.20f, 0.20f, 0.35f);

        if (bStyleDark_)
        {
            for (int i = 0; i < ImGuiCol_COUNT; i++)
            {
                ImVec4& col = style.Colors[i];
                float   H, S, V;
                ImGui::ColorConvertRGBtoHSV(col.x, col.y, col.z, H, S, V);

                if (S < 0.1f) { V = 1.0f - V; }
                ImGui::ColorConvertHSVtoRGB(H, S, V, col.x, col.y, col.z);
                if (col.w < 1.00f) { col.w *= alpha_; }
            }
        }
        else
        {
            for (int i = 0; i < ImGuiCol_COUNT; i++)
            {
                ImVec4& col = style.Colors[i];
                if (col.w < 1.00f)
                {
                    col.x *= alpha_;
                    col.y *= alpha_;
                    col.z *= alpha_;
                    col.w *= alpha_;
                }
            }
        }
    }

    ImGui_ImplVortex_Data* GetBackendData()
    {
        return reinterpret_cast<ImGui_ImplVortex_Data*>(
            ImGui::GetIO().BackendPlatformUserData);
    }
    inline static ImGuiViewportData* GetViewportData(ImGuiViewport* viewport)
    {
        return reinterpret_cast<ImGuiViewportData*>(viewport->PlatformUserData);
    }
    ImGuiKey ToImGuiKey(KeyCode key)
    {
        using Input::KeyCode;

        switch (key)
        {
            case KeyCode::eTab: return ImGuiKey_Tab;
            case KeyCode::eLeft: return ImGuiKey_LeftArrow;
            case KeyCode::eRight: return ImGuiKey_RightArrow;
            case KeyCode::eUp: return ImGuiKey_UpArrow;
            case KeyCode::eDown: return ImGuiKey_DownArrow;
            case KeyCode::ePageUp: return ImGuiKey_PageUp;
            case KeyCode::ePageDown: return ImGuiKey_PageDown;
            case KeyCode::eHome: return ImGuiKey_Home;
            case KeyCode::eEnd: return ImGuiKey_End;
            case KeyCode::eInsert: return ImGuiKey_Insert;
            case KeyCode::eDelete: return ImGuiKey_Delete;
            case KeyCode::eBackspace: return ImGuiKey_Backspace;
            case KeyCode::eSpace: return ImGuiKey_Space;
            case KeyCode::eReturn: return ImGuiKey_Enter;
            case KeyCode::eEscape: return ImGuiKey_Escape;
            case KeyCode::eApostrophe: return ImGuiKey_Apostrophe;
            case KeyCode::eComma: return ImGuiKey_Comma;
            case KeyCode::eHyphen: return ImGuiKey_Minus;
            case KeyCode::ePeriod: return ImGuiKey_Period;
            case KeyCode::eSlash: return ImGuiKey_Slash;
            case KeyCode::eSemicolon: return ImGuiKey_Semicolon;
            case KeyCode::eEqual: return ImGuiKey_Equal;
            case KeyCode::eLBracket: return ImGuiKey_LeftBracket;
            case KeyCode::eBackslash: return ImGuiKey_Backslash;
            case KeyCode::eRBracket: return ImGuiKey_RightBracket;
            case KeyCode::eTilde: return ImGuiKey_GraveAccent;
            case KeyCode::eCapsLock: return ImGuiKey_CapsLock;
            case KeyCode::eScrollLock: return ImGuiKey_ScrollLock;
            case KeyCode::eNumLock: return ImGuiKey_NumLock;
            case KeyCode::ePrintScreen: return ImGuiKey_PrintScreen;
            case KeyCode::ePause: return ImGuiKey_Pause;
            case KeyCode::eNumpad0: return ImGuiKey_Keypad0;
            case KeyCode::eNumpad1: return ImGuiKey_Keypad1;
            case KeyCode::eNumpad2: return ImGuiKey_Keypad2;
            case KeyCode::eNumpad3: return ImGuiKey_Keypad3;
            case KeyCode::eNumpad4: return ImGuiKey_Keypad4;
            case KeyCode::eNumpad5: return ImGuiKey_Keypad5;
            case KeyCode::eNumpad6: return ImGuiKey_Keypad6;
            case KeyCode::eNumpad7: return ImGuiKey_Keypad7;
            case KeyCode::eNumpad8: return ImGuiKey_Keypad8;
            case KeyCode::eNumpad9: return ImGuiKey_Keypad9;
            case KeyCode::eDecimal: return ImGuiKey_KeypadDecimal;
            case KeyCode::eDivide: return ImGuiKey_KeypadDivide;
            case KeyCode::eMultiply: return ImGuiKey_KeypadMultiply;
            case KeyCode::eSubtract: return ImGuiKey_KeypadSubtract;
            case KeyCode::eAdd: return ImGuiKey_KeypadAdd;
            case KeyCode::eEnter: return ImGuiKey_KeypadEnter;
            // case KeyCode: return ImGuiKey_KeypadEqual;
            case KeyCode::eLShift: return ImGuiKey_LeftShift;
            case KeyCode::eLCtrl: return ImGuiKey_LeftCtrl;
            case KeyCode::eLAlt: return ImGuiKey_LeftAlt;
            case KeyCode::eLSystem: return ImGuiKey_LeftSuper;
            case KeyCode::eRShift: return ImGuiKey_RightShift;
            case KeyCode::eRCtrl: return ImGuiKey_RightCtrl;
            case KeyCode::eRAlt: return ImGuiKey_RightAlt;
            case KeyCode::eRSystem: return ImGuiKey_RightSuper;
            case KeyCode::eMenu: return ImGuiKey_Menu;
            case KeyCode::eNum0: return ImGuiKey_0;
            case KeyCode::eNum1: return ImGuiKey_1;
            case KeyCode::eNum2: return ImGuiKey_2;
            case KeyCode::eNum3: return ImGuiKey_3;
            case KeyCode::eNum4: return ImGuiKey_4;
            case KeyCode::eNum5: return ImGuiKey_5;
            case KeyCode::eNum6: return ImGuiKey_6;
            case KeyCode::eNum7: return ImGuiKey_7;
            case KeyCode::eNum8: return ImGuiKey_8;
            case KeyCode::eNum9: return ImGuiKey_9;
            case KeyCode::eA: return ImGuiKey_A;
            case KeyCode::eB: return ImGuiKey_B;
            case KeyCode::eC: return ImGuiKey_C;
            case KeyCode::eD: return ImGuiKey_D;
            case KeyCode::eE: return ImGuiKey_E;
            case KeyCode::eF: return ImGuiKey_F;
            case KeyCode::eG: return ImGuiKey_G;
            case KeyCode::eH: return ImGuiKey_H;
            case KeyCode::eI: return ImGuiKey_I;
            case KeyCode::eJ: return ImGuiKey_J;
            case KeyCode::eK: return ImGuiKey_K;
            case KeyCode::eL: return ImGuiKey_L;
            case KeyCode::eM: return ImGuiKey_M;
            case KeyCode::eN: return ImGuiKey_N;
            case KeyCode::eO: return ImGuiKey_O;
            case KeyCode::eP: return ImGuiKey_P;
            case KeyCode::eQ: return ImGuiKey_Q;
            case KeyCode::eR: return ImGuiKey_R;
            case KeyCode::eS: return ImGuiKey_S;
            case KeyCode::eT: return ImGuiKey_T;
            case KeyCode::eU: return ImGuiKey_U;
            case KeyCode::eV: return ImGuiKey_V;
            case KeyCode::eW: return ImGuiKey_W;
            case KeyCode::eX: return ImGuiKey_X;
            case KeyCode::eY: return ImGuiKey_Y;
            case KeyCode::eZ: return ImGuiKey_Z;
            case KeyCode::eF1: return ImGuiKey_F1;
            case KeyCode::eF2: return ImGuiKey_F2;
            case KeyCode::eF3: return ImGuiKey_F3;
            case KeyCode::eF4: return ImGuiKey_F4;
            case KeyCode::eF5: return ImGuiKey_F5;
            case KeyCode::eF6: return ImGuiKey_F6;
            case KeyCode::eF7: return ImGuiKey_F7;
            case KeyCode::eF8: return ImGuiKey_F8;
            case KeyCode::eF9: return ImGuiKey_F9;
            case KeyCode::eF10: return ImGuiKey_F10;
            case KeyCode::eF11: return ImGuiKey_F11;
            case KeyCode::eF12: return ImGuiKey_F12;
            case KeyCode::eF13: return ImGuiKey_F13;
            case KeyCode::eF14: return ImGuiKey_F14;
            case KeyCode::eF15: return ImGuiKey_F15;
            /*case KeyCode::eF16: return ImGuiKey_F16;
            case KeyCode::eF17: return ImGuiKey_F17;
            case KeyCode::eF18: return ImGuiKey_F18;
            case KeyCode::eF19: return ImGuiKey_F19;
            case KeyCode::eF20: return ImGuiKey_F20;
            case KeyCode::eF21: return ImGuiKey_F21;
            case KeyCode::eF22: return ImGuiKey_F22;
            case KeyCode::eF23: return ImGuiKey_F23;
            case KeyCode::eF24: return ImGuiKey_F24;*/
            default: break;
        }

        return ImGuiKey_None;
    }
    ImGuiMouseButton ToImGuiMouseButton(MouseCode button)
    {
        switch (button)
        {
            case MouseCode::eLeft: return ImGuiMouseButton_Left;
            case MouseCode::eRight: return ImGuiMouseButton_Right;
            case MouseCode::eMiddle: return ImGuiMouseButton_Middle;

            default: break;
        }

        return ImGuiMouseButton_COUNT;
    }
    void UpdateKeyModifiers(KeyCode key, bool pressed)
    {
        ImGuiIO& io = ImGui::GetIO();

        switch (key)
        {
            case KeyCode::eLCtrl:
            case KeyCode::eRCtrl: io.AddKeyEvent(ImGuiMod_Ctrl, pressed); break;
            case KeyCode::eLAlt:
            case KeyCode::eRAlt: io.AddKeyEvent(ImGuiMod_Alt, pressed); break;
            case KeyCode::eLShift:
            case KeyCode::eRShift:
                io.AddKeyEvent(ImGuiMod_Shift, pressed);
                break;
            case KeyCode::eLSystem:
            case KeyCode::eRSystem:
                io.AddKeyEvent(ImGuiMod_Super, pressed);
                break;

            default: break;
        }
    }

    void VulkanImGuiLayer::OnAttach()
    {
        auto& window = Application::Get()->GetWindow();
        auto  swapChain
            = std::dynamic_pointer_cast<VulkanSwapChain>(window.GetSwapChain());
        vk::Queue graphicsQueue
            = VulkanRenderer::GetDevice().GetGraphicsQueue();

        vk::DescriptorPoolSize poolSizes[]
            = {{vk::DescriptorType::eSampler, 1000},
               {vk::DescriptorType::eCombinedImageSampler, 1000},
               {vk::DescriptorType::eSampledImage, 1000},
               {vk::DescriptorType::eStorageImage, 1000},
               {vk::DescriptorType::eUniformTexelBuffer, 1000},
               {vk::DescriptorType::eStorageTexelBuffer, 1000},
               {vk::DescriptorType::eUniformBuffer, 1000},
               {vk::DescriptorType::eStorageBuffer, 1000},
               {vk::DescriptorType::eUniformBufferDynamic, 1000},
               {vk::DescriptorType::eStorageBufferDynamic, 1000},
               {vk::DescriptorType::eInputAttachment, 1000}};

        vk::DescriptorPoolCreateInfo poolInfo = {};
        poolInfo.sType   = vk::StructureType::eDescriptorPoolCreateInfo;
        poolInfo.flags   = vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet;
        poolInfo.maxSets = 1000;
        poolInfo.poolSizeCount = static_cast<uint32_t>(std::size(poolSizes));
        poolInfo.pPoolSizes    = poolSizes;

        vk::Device device      = VulkanRenderer::GetDevice();
        VkCall(device.createDescriptorPool(&poolInfo, VK_NULL_HANDLE,
                                           &m_DescriptorPool));

        // Setup Dear ImGui context
        IMGUI_CHECKVERSION();
        VtCoreTrace("ImGui: Creating context...");
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();
        VtCoreAssert(io.BackendPlatformUserData == nullptr
                     && "Already initialized a platform backend!");

        io.ConfigFlags
            |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
        io.ConfigFlags
            |= ImGuiConfigFlags_NavEnableGamepad; // Enable Gamepad Controls
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable; // Enable Docking

        if (Window::GetWindowSubsystem() != WindowSubsystem::eWayland)
            io.ConfigFlags
                |= ImGuiConfigFlags_ViewportsEnable; // Enable Multi-Viewport
                                                     // / Platform Windows

        ImGui::StyleColorsDark();
        f32 fontSize   = 16.8f;
        io.FontDefault = io.Fonts->AddFontFromFileTTF(
            "assets/fonts/JetBrainsMonoNerdFont-SemiBoldItalic.ttf", fontSize);
        SetupImGuiStyle(true, 0.94f);

        // When viewports are enabled we tweak WindowRounding/WindowBg so
        // platform windows can look identical to regular ones.
        ImGuiStyle& style = ImGui::GetStyle();
        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        {
            style.WindowRounding              = 0.0f;
            style.Colors[ImGuiCol_WindowBg].w = 1.0f;
        }

        auto indices
            = VulkanRenderer::GetPhysicalDevice().GetQueueFamilyIndices();

        // Setup Platform/Renderer backends
        // Setup backend capabilities flags
        ImGui_ImplVortex_Data* bd  = new ImGui_ImplVortex_Data;
        io.BackendPlatformUserData = reinterpret_cast<void*>(bd);
        io.BackendPlatformName     = "Vortex-Engine";
        // io.BackendFlags
        //     |= ImGuiBackendFlags_HasMouseCursors; // We can honor
        //  GetMouseCursor() values
        //  (optional)
        io.BackendFlags
            |= ImGuiBackendFlags_HasSetMousePos; // We can honor
                                                 // io.WantSetMousePos requests
                                                 // (optional, rarely used)

        if (Window::GetWindowSubsystem() != WindowSubsystem::eWayland)
            io.BackendFlags
                |= ImGuiBackendFlags_PlatformHasViewports; // We can create
                                                           // multi-viewports on
                                                           // the Platform side
                                                           // (optional)
        io.BackendFlags
            |= ImGuiBackendFlags_HasMouseHoveredViewport; // We can call
                                                          // io.AddMouseViewportEvent()
                                                          // with correct data
                                                          // (optional)

        bd->MainWindow         = &window;
        bd->Time               = 0.0;
        bd->WantUpdateMonitors = true;

        // TODO(v1tr10l7): Clipboard
        io.SetClipboardTextFn  = [](void*, const char*) {};
        io.GetClipboardTextFn  = [](void*) -> const char* { return ""; };
        io.ClipboardUserData   = &window;
        CreateCursors();
        SetUpEvents();
        UpdateMonitors();
        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
            InitializePlatformInterface();

        // Set platform dependent data in viewport
        ImGuiViewport* mainViewport    = ImGui::GetMainViewport();
        auto           data            = new ImGuiViewportData;
        data->Window                   = bd->MainWindow;
        data->WindowOwned              = false;
        mainViewport->PlatformUserData = data;
        mainViewport->PlatformHandle = reinterpret_cast<void*>(bd->MainWindow);

        ImGui_ImplVulkan_InitInfo initInfo = {};
        initInfo.Instance = vk::Instance(VulkanRenderer::GetVulkanInstance());
        initInfo.PhysicalDevice
            = vk::PhysicalDevice(VulkanRenderer::GetPhysicalDevice());
        initInfo.Device         = device;
        initInfo.QueueFamily    = indices.Graphics.value();
        initInfo.Queue          = graphicsQueue;
        initInfo.PipelineCache  = VK_NULL_HANDLE;
        initInfo.DescriptorPool = m_DescriptorPool;
        initInfo.RenderPass     = swapChain->GetRenderPass();
        initInfo.Subpass        = 0;
        u32 frameCount = static_cast<u32>(swapChain->GetFrames().size());
        initInfo.MinImageCount = frameCount;
        initInfo.ImageCount    = frameCount;
        initInfo.MSAASamples   = VK_SAMPLE_COUNT_1_BIT;
        initInfo.Allocator     = VK_NULL_HANDLE;
        initInfo.CheckVkResultFn
            = [](VkResult result) -> void { VkCall(vk::Result(result)); };
        ImGui_ImplVulkan_Init(&initInfo);

        vk::CommandPoolCreateInfo commandPoolInfo{};
        commandPoolInfo.sType = vk::StructureType::eCommandPoolCreateInfo;
        commandPoolInfo.pNext = VK_NULL_HANDLE;
        commandPoolInfo.flags
            = vk::CommandPoolCreateFlagBits::eResetCommandBuffer;
        commandPoolInfo.queueFamilyIndex = VulkanRenderer::GetPhysicalDevice()
                                               .GetQueueFamilyIndices()
                                               .Graphics.value();
        VkCall(device.createCommandPool(&commandPoolInfo, VK_NULL_HANDLE,
                                        &m_CommandPool));

        m_CommandBuffers.resize(frameCount);
        vk::CommandBufferAllocateInfo bufferAlloc{};
        bufferAlloc.sType       = vk::StructureType::eCommandBufferAllocateInfo;
        bufferAlloc.commandPool = m_CommandPool;
        bufferAlloc.level       = vk::CommandBufferLevel::eSecondary;
        bufferAlloc.commandBufferCount = frameCount;

        VkCall(device.allocateCommandBuffers(&bufferAlloc,
                                             m_CommandBuffers.data()));
    }
    void VulkanImGuiLayer::OnDetach()
    {
        VtCoreTrace("Detaching VulkanImGuiLayer...");

        /*WindowEvents::KeyPressedEvent -= OnKeyPressed;
        WindowEvents::KeyReleasedEvent -= OnKeyReleased;
        WindowEvents::KeyTypedEvent -= OnKeyTyped;

        WindowEvents::MouseButtonPressedEvent -= OnMouseButtonPressed;
        WindowEvents::MouseButtonReleasedEvent -= OnMouseButtonReleased;
        WindowEvents::MouseScrolledEvent -= OnMouseScrolled;
        WindowEvents::MouseMovedEvent -= OnMouseMoved;
        WindowEvents::MouseEnteredEvent -= OnMouseEntered;
        WindowEvents::MouseLeftEvent -= OnMouseLeft;

        WindowEvents::WindowMovedEvent -= OnWindowMoved;
        WindowEvents::WindowResizedEvent -= OnWindowResized;
        WindowEvents::WindowFocusedEvent -= OnWindowFocusGained;
        WindowEvents::WindowFocusLostEvent -= OnWindowFocusLost;
        WindowEvents::WindowClosedEvent -= OnWindowClosed;

        MonitorEvents::MonitorStateChangedEvent -= OnMonitorStateUpdate;*/
        vk::Device device = VulkanRenderer::GetDevice();
        device.waitIdle();

        device.destroyCommandPool(m_CommandPool);

        auto mainViewport = ImGui::GetMainViewport();
        delete reinterpret_cast<ImGuiViewportData*>(
            mainViewport->PlatformUserData);
        mainViewport->PlatformUserData = nullptr;
        mainViewport->PlatformHandle   = nullptr;
        ImGui_ImplVulkan_Shutdown();
        ImGui_ImplVortex_Data* bd = GetBackendData();
        VtCoreAssert(
            bd != nullptr
            && "No platform backend to shutdown, or already shutdown?");
        ImGuiIO& io = ImGui::GetIO();

        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        {
            VtCoreTrace("VulkanImGuiLayer: Destroying platform windows...");
            ImGui::DestroyPlatformWindows();
        }
        io.BackendPlatformName     = nullptr;
        io.BackendPlatformUserData = nullptr;
        io.BackendFlags &= ~(ImGuiBackendFlags_HasMouseCursors
                             | ImGuiBackendFlags_HasSetMousePos
                             | ImGuiBackendFlags_HasGamepad
                             | ImGuiBackendFlags_PlatformHasViewports
                             | ImGuiBackendFlags_HasMouseHoveredViewport);
        delete bd;
        ImGui::DestroyContext();

        vkDestroyDescriptorPool(device, m_DescriptorPool, VK_NULL_HANDLE);
    }

    void VulkanImGuiLayer::Begin()
    {
        ImGui_ImplVulkan_NewFrame();

        ImGuiIO& io              = ImGui::GetIO();
        auto     bd              = GetBackendData();
        auto     windowSize      = bd->MainWindow->GetSize();

        auto     framebufferSize = bd->MainWindow->GetFramebufferSize();
        io.DisplaySize           = ImVec2(static_cast<f32>(windowSize.x),
                                          static_cast<f32>(windowSize.y));
        if (windowSize.x > 0 && windowSize.y > 0)
            io.DisplayFramebufferScale
                = ImVec2(static_cast<f32>(framebufferSize.x)
                             / static_cast<f32>(windowSize.x),
                         static_cast<f32>(framebufferSize.y)
                             / static_cast<f32>(windowSize.y));
        if (bd->WantUpdateMonitors) UpdateMonitors();

        f64 currentTime = Time::GetCurrentTime();
        if (currentTime <= bd->Time) currentTime = bd->Time + 0.00001f;
        io.DeltaTime = bd->Time > 0.0 ? (f32)(currentTime - bd->Time)
                                      : (f32)(1.0f / 60.0f);
        bd->Time     = currentTime;
        UpdateMouseData();
        UpdateMouseCursor();

        // TODO(v1tr10l7): Gamepad support for imgui
        ImGui::NewFrame();
        ImGuizmo::BeginFrame();
    }
    void VulkanImGuiLayer::End()
    {
        auto& window = Application::Get()->GetWindow();
        auto  swapChain
            = std::dynamic_pointer_cast<VulkanSwapChain>(window.GetSwapChain());
        vk::CommandBuffer commandBuffer = swapChain->GetCurrentCommandBuffer();
        ImGui::Render();

#if 0
        auto         currentImageIndex = swapChain.GetCurrentImageIndex();
        vk::Extent2D extent            = swapChain.GetExtent();

        vk::RenderingAttachmentInfoKHR colorAttachmentInfo{};
        colorAttachmentInfo.sType
            = vk::StructureType::eRenderingAttachmentInfoKHR;
        colorAttachmentInfo.imageView
            = swapChain.GetFrames()[currentImageIndex].ImageView;
        colorAttachmentInfo.imageLayout = vk::ImageLayout::eAttachmentOptimal;
        colorAttachmentInfo.loadOp      = vk::AttachmentLoadOp::eLoad;
        colorAttachmentInfo.storeOp     = vk::AttachmentStoreOp::eStore;

        vk::RenderingInfoKHR renderingInfo{};
        renderingInfo.sType             = vk::StructureType::eRenderingInfoKHR;
        renderingInfo.renderArea.offset = vk::Offset2D(0, 0);
        renderingInfo.renderArea.extent = extent;
        renderingInfo.layerCount        = 1;
        renderingInfo.colorAttachmentCount = 1;
        renderingInfo.pColorAttachments    = &colorAttachmentInfo;

        commandBuffer.beginRenderingKHR(&renderingInfo);
#endif

        ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), commandBuffer);

#if 0
        commandBuffer.endRenderingKHR();
#endif

        ImGuiIO io = ImGui::GetIO();
        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        {
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
        }
    }

    void VulkanImGuiLayer::OnUpdate() {}
    void VulkanImGuiLayer::OnRender() {}
    void VulkanImGuiLayer::OnImGuiRender() {}

    void VulkanImGuiLayer::CreateCursors()
    {
        // TODO(v1tr10l7): VulkanImGuiLayer::CreateCursors()
    }
    void VulkanImGuiLayer::SetUpEvents()
    {
        WindowEvents::KeyPressedEvent += OnKeyPressed;
        WindowEvents::KeyReleasedEvent += OnKeyReleased;
        WindowEvents::KeyTypedEvent += OnKeyTyped;

        WindowEvents::MouseButtonPressedEvent += OnMouseButtonPressed;
        WindowEvents::MouseButtonReleasedEvent += OnMouseButtonReleased;
        WindowEvents::MouseScrolledEvent += OnMouseScrolled;
        WindowEvents::MouseMovedEvent += OnMouseMoved;
        WindowEvents::MouseEnteredEvent += OnMouseEntered;
        WindowEvents::MouseLeftEvent += OnMouseLeft;

        WindowEvents::WindowMovedEvent += OnWindowMoved;
        WindowEvents::WindowResizedEvent += OnWindowResized;
        WindowEvents::WindowFocusedEvent += OnWindowFocusGained;
        WindowEvents::WindowFocusLostEvent += OnWindowFocusLost;
        WindowEvents::WindowClosedEvent += OnWindowClosed;

        MonitorEvents::MonitorStateChangedEvent += OnMonitorStateUpdate;
    }
    void VulkanImGuiLayer::InitializePlatformInterface()
    {
        ImGuiPlatformIO& io            = ImGui::GetPlatformIO();
        io.Platform_CreateWindow       = CreateWindow;
        io.Platform_DestroyWindow      = DestroyWindow;
        io.Platform_ShowWindow         = ShowWindow;
        io.Platform_SetWindowPos       = SetWindowPosition;
        io.Platform_GetWindowPos       = GetWindowPosition;
        io.Platform_SetWindowSize      = SetWindowSize;
        io.Platform_GetWindowSize      = GetWindowSize;
        io.Platform_SetWindowFocus     = RequestWindowFocus;
        io.Platform_GetWindowFocus     = IsWindowFocused;
        io.Platform_GetWindowMinimized = IsWindowMinimized;
        io.Platform_SetWindowTitle     = SetWindowTitle;
        io.Platform_SetWindowAlpha     = SetWindowOpacity;
        io.Platform_RenderWindow       = [](ImGuiViewport*, void*) {};
        io.Platform_SwapBuffers        = [](ImGuiViewport*, void*) {};
        io.Platform_CreateVkSurface    = CreateVulkanSurface;
    }

    void VulkanImGuiLayer::UpdateMouseData()
    {
        ImGui_ImplVortex_Data* bd              = GetBackendData();
        ImGuiIO&               io              = ImGui::GetIO();
        ImGuiPlatformIO&       platformIO      = ImGui::GetPlatformIO();

        ImGuiID                mouseViewportID = 0;
        const ImVec2           mousePrevPos    = io.MousePos;
        for (int n = 0; n < platformIO.Viewports.Size; n++)
        {
            ImGuiViewport* viewport = platformIO.Viewports[n];
            auto           data     = GetViewportData(viewport);

            if (data->Window->IsFocused())
            {
                if (io.WantSetMousePos)
                    data->Window->SetCursorPosition(
                        Vec2d((f64)(mousePrevPos.x - viewport->Pos.x),
                              (f64)(mousePrevPos.y - viewport->Pos.y)));

                if (!bd->MouseWindow)
                {
                    auto cursorPos = data->Window->GetCursorPosition();
                    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
                    {
                        // Single viewport mode: mouse position in client window
                        // coordinates (io.MousePos is (0,0) when the mouse is
                        // on the upper-left corner of the app window)
                        // Multi-viewport mode: mouse position in OS absolute
                        // coordinates (io.MousePos is (0,0) when the mouse is
                        // on the upper-left of the primary monitor)
                        auto windowPos = data->Window->GetPosition();
                        cursorPos += windowPos;
                    }
                    bd->LastValidMousePos
                        = ImVec2((float)cursorPos.x, (float)cursorPos.y);
                    io.AddMousePosEvent((float)cursorPos.x, (float)cursorPos.y);
                }
            }

            if (data->Window->IsHovered()) mouseViewportID = viewport->ID;
        }

        if (io.BackendFlags & ImGuiBackendFlags_HasMouseHoveredViewport)
            io.AddMouseViewportEvent(mouseViewportID);
    }
    void VulkanImGuiLayer::UpdateMouseCursor()
    {
        ImGuiIO& io = ImGui::GetIO();
        if ((io.ConfigFlags & ImGuiConfigFlags_NoMouseCursorChange)) return;

        ImGuiMouseCursor imguiCursor = ImGui::GetMouseCursor();
        ImGuiPlatformIO& platformIO  = ImGui::GetPlatformIO();
        for (i32 n = 0; n < platformIO.Viewports.Size; n++)
        {
            auto window = reinterpret_cast<Window*>(
                platformIO.Viewports[n]->PlatformHandle);

            if (imguiCursor == ImGuiMouseCursor_None || io.MouseDrawCursor)
                window->HideCursor();
            // else
            //  TODO(v1tr10l7): Update cursor
        }
    }
    void VulkanImGuiLayer::UpdateMonitors()
    {
        ImGui_ImplVortex_Data* bd         = GetBackendData();
        ImGuiPlatformIO&       platformIO = ImGui::GetPlatformIO();
        bd->WantUpdateMonitors            = false;

        i32 monitorCount                  = 1;
        if (monitorCount == 0) return;

        // TODO(v1tr10l7): Temporary
        platformIO.Monitors.resize(0);
        ImGuiPlatformMonitor monitor{};
        monitor.MainPos = monitor.WorkPos
            = ImVec2(static_cast<f32>(0), static_cast<f32>(0));
        monitor.MainSize = monitor.WorkSize = ImVec2(1600, 900);
        platformIO.Monitors.push_back(monitor);
    }

    bool VulkanImGuiLayer::OnKeyPressed(Window* window, KeyCode key,
                                        u32 repeatCount)
    {
        ImGui_ImplVortex_Data* bd = GetBackendData();
        if (repeatCount) return false;
        UpdateKeyModifiers(key, true);

        usize keycode = std::to_underlying(key);

        if (keycode < std::size(bd->KeyOwnerWindows))
            bd->KeyOwnerWindows[keycode] = window;

        ImGuiIO& io       = ImGui::GetIO();
        ImGuiKey imguiKey = ToImGuiKey(key);
        io.AddKeyEvent(imguiKey, true);
        io.SetKeyEventNativeData(imguiKey, static_cast<i32>(key), 0);

        return false;
    }
    bool VulkanImGuiLayer::OnKeyReleased(Window* window, KeyCode key)
    {
        ImGui_ImplVortex_Data* bd = GetBackendData();
        UpdateKeyModifiers(key, false);

        usize keycode = std::to_underlying(key);
        if (keycode < std::size(bd->KeyOwnerWindows))
            bd->KeyOwnerWindows[keycode] = window;

        ImGuiIO& io       = ImGui::GetIO();
        ImGuiKey imguiKey = ToImGuiKey(key);
        io.AddKeyEvent(imguiKey, false);
        io.SetKeyEventNativeData(imguiKey, static_cast<i32>(key), 0);

        return false;
    }
    bool VulkanImGuiLayer::OnKeyTyped(Window*, u32 c)
    {
        ImGuiIO& io = ImGui::GetIO();
        io.AddInputCharacter(c);

        return false;
    }

    bool VulkanImGuiLayer::OnMouseButtonPressed(Window*, MouseCode button)
    {
        ImGuiIO& io = ImGui::GetIO();
        if (std::to_underlying(button) < 0
            || std::to_underlying(button) >= ImGuiMouseButton_COUNT)
            return false;

        io.AddMouseButtonEvent(ToImGuiMouseButton(button), true);
        return false;
    }
    bool VulkanImGuiLayer::OnMouseButtonReleased(Window*, MouseCode button)
    {
        ImGuiIO& io = ImGui::GetIO();
        if (std::to_underlying(button) < 0
            || std::to_underlying(button) >= ImGuiMouseButton_COUNT)
            return false;

        io.AddMouseButtonEvent(ToImGuiMouseButton(button), false);
        return false;
    }
    bool VulkanImGuiLayer::OnMouseScrolled(Window*, f64 xoffset, f64 yoffset)
    {
        ImGuiIO& io = ImGui::GetIO();
        io.AddMouseWheelEvent(static_cast<f32>(xoffset),
                              static_cast<f32>(yoffset));

        return false;
    }
    bool VulkanImGuiLayer::OnMouseMoved(Window* window, f64 xoffset,
                                        f64 yoffset)
    {
        ImGui_ImplVortex_Data* bd = GetBackendData();

        ImGuiIO&               io = ImGui::GetIO();
        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        {
            Vec2i pos = window->GetPosition();
            xoffset += pos.x;
            yoffset += pos.y;
        }

        f32 x = static_cast<f32>(xoffset);
        f32 y = static_cast<f32>(yoffset);

        io.AddMousePosEvent(x, y);
        bd->LastValidMousePos = ImVec2(x, y);
        return false;
    }
    bool VulkanImGuiLayer::OnMouseEntered(Window* window)
    {
        ImGui_ImplVortex_Data* bd = GetBackendData();

        ImGuiIO&               io = ImGui::GetIO();
        bd->MouseWindow           = window;
        io.AddMousePosEvent(bd->LastValidMousePos.x, bd->LastValidMousePos.y);
        return false;
    }
    bool VulkanImGuiLayer::OnMouseLeft(Window* window)
    {
        ImGui_ImplVortex_Data* bd = GetBackendData();

        if (bd->MouseWindow == window)
        {
            ImGuiIO& io           = ImGui::GetIO();

            bd->LastValidMousePos = io.MousePos;
            bd->MouseWindow       = nullptr;
            io.AddMousePosEvent(-FLT_MAX, -FLT_MAX);
        }
        return false;
    }

    bool VulkanImGuiLayer::OnWindowMoved(Window* window, i32, i32)
    {
        if (ImGuiViewport* viewport
            = ImGui::FindViewportByPlatformHandle(window))
        {
            if (ImGuiViewportData* vd
                = (ImGuiViewportData*)viewport->PlatformUserData)
            {
                bool ignoreEvent = (ImGui::GetFrameCount()
                                    <= vd->IgnoreWindowPosEventFrame + 1);
                if (ignoreEvent) return false;
            }
            viewport->PlatformRequestMove = true;
        }
        return false;
    }
    bool VulkanImGuiLayer::OnWindowResized(Window* window, i32, i32)
    {
        if (ImGuiViewport* viewport
            = ImGui::FindViewportByPlatformHandle(window))
        {
            if (ImGuiViewportData* vd
                = (ImGuiViewportData*)viewport->PlatformUserData)
            {
                bool ignoreEvent = (ImGui::GetFrameCount()
                                    <= vd->IgnoreWindowSizeEventFrame + 1);
                if (ignoreEvent) return false;
            }
            viewport->PlatformRequestResize = true;
        }
        return false;
    }
    bool VulkanImGuiLayer::OnWindowFocusGained(Window*)
    {
        ImGuiIO& io = ImGui::GetIO();
        io.AddFocusEvent(true);
        return false;
    }
    bool VulkanImGuiLayer::OnWindowFocusLost(Window*)
    {
        ImGuiIO& io = ImGui::GetIO();
        io.AddFocusEvent(false);
        return false;
    }
    bool VulkanImGuiLayer::OnWindowClosed(Window* window)
    {
        if (ImGuiViewport* viewport
            = ImGui::FindViewportByPlatformHandle(window))
            viewport->PlatformRequestClose = true;
        return false;
    }

    bool VulkanImGuiLayer::OnMonitorStateUpdate(Monitor*, MonitorState)
    {
        auto bd                = GetBackendData();
        bd->WantUpdateMonitors = true;

        return false;
    }

    void VulkanImGuiLayer::CreateWindow(ImGuiViewport* viewport)
    {
        ImGuiViewportData* data    = new ImGuiViewportData;
        viewport->PlatformUserData = data;

        WindowSpecification specs{};
        specs.Visible     = false;
        specs.Focused     = false;
        specs.FocusOnShow = false;
        specs.Decorated = !(viewport->Flags & ImGuiViewportFlags_NoDecoration);
        specs.AlwaysOnTop      = (viewport->Flags & ImGuiViewportFlags_TopMost);
        specs.VideoMode.Width  = static_cast<i32>(viewport->Size.x);
        specs.VideoMode.Height = static_cast<i32>(viewport->Size.y);
        specs.Title            = "No Title Yet";
        Window* window         = Window::CreateForImGui(specs);

        data->Window           = window;
        data->WindowOwned      = true;
        viewport->PlatformHandle = (void*)data->Window;
        data->Window->SetPosition({viewport->Pos.x, viewport->Pos.y});
    }
    void VulkanImGuiLayer::DestroyWindow(ImGuiViewport* viewport)
    {
        ImGui_ImplVortex_Data* bd = GetBackendData();
        if (auto data = GetViewportData(viewport))
        {
            if (data->WindowOwned
                && data->Window != &Application::Get()->GetWindow())
            {
                // Release any keys that were pressed in the window being
                // destroyed and are still held down, because we will not
                // receive any release events after window is destroyed.
                for (usize i = 0; i < std::size(bd->KeyOwnerWindows); i++)
                    if (bd->KeyOwnerWindows[i] == data->Window)
                        OnKeyReleased(data->Window,
                                      static_cast<Input::KeyCode>(i));

                delete data->Window;
            }
            delete data;
        }
        viewport->PlatformUserData = viewport->PlatformHandle = nullptr;
    }
    void VulkanImGuiLayer::ShowWindow(ImGuiViewport* viewport)
    {
        GetViewportData(viewport)->Window->SetVisible(true);
    }
    ImVec2 VulkanImGuiLayer::GetWindowPosition(ImGuiViewport* viewport)
    {
        auto position = GetViewportData(viewport)->Window->GetPosition();

        return ImVec2(static_cast<f32>(position.x),
                      static_cast<f32>(position.y));
    }
    void VulkanImGuiLayer::SetWindowPosition(ImGuiViewport* viewport,
                                             ImVec2         position)
    {
        auto data                       = GetViewportData(viewport);
        data->IgnoreWindowPosEventFrame = ImGui::GetFrameCount();

        data->Window->SetPosition({position.x, position.y});
    }
    ImVec2 VulkanImGuiLayer::GetWindowSize(ImGuiViewport* viewport)
    {
        auto size = GetViewportData(viewport)->Window->GetSize();

        return ImVec2(static_cast<f32>(size.x), static_cast<f32>(size.y));
    }
    void VulkanImGuiLayer::SetWindowSize(ImGuiViewport* viewport, ImVec2 size)
    {
        auto data                        = GetViewportData(viewport);
        data->IgnoreWindowSizeEventFrame = ImGui::GetFrameCount();

        data->Window->SetSize(Vec2(size.x, size.y));
    }
    void VulkanImGuiLayer::SetWindowTitle(ImGuiViewport* viewport,
                                          const char*    title)
    {
        GetViewportData(viewport)->Window->SetTitle(title);
    }
    void VulkanImGuiLayer::RequestWindowFocus(ImGuiViewport* viewport)
    {
        GetViewportData(viewport)->Window->RequestFocus();
    }
    bool VulkanImGuiLayer::IsWindowFocused(ImGuiViewport* viewport)
    {
        return GetViewportData(viewport)->Window->IsFocused();
    }
    bool VulkanImGuiLayer::IsWindowMinimized(ImGuiViewport* viewport)
    {
        return GetViewportData(viewport)->Window->IsMinimized();
    }
    void VulkanImGuiLayer::SetWindowOpacity(ImGuiViewport* viewport,
                                            f32            opacity)
    {
        GetViewportData(viewport)->Window->SetOpacity(opacity);
    }
    i32 VulkanImGuiLayer::CreateVulkanSurface(ImGuiViewport* viewport, ImU64,
                                              const void*, ImU64* outVkSurface)
    {
        ImGuiViewportData* data = GetViewportData(viewport);
        auto          surface   = reinterpret_cast<VkSurfaceKHR*>(outVkSurface);

        VulkanSurface surf;
        surf.Create(data->Window, VulkanRenderer::GetPhysicalDevice());
        *surface = vk::SurfaceKHR(surf);

        return static_cast<i32>(VK_SUCCESS);
    }
}; // namespace Vortex
