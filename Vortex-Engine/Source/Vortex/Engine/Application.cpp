/*
 * Created by v1tr10l7 on 17.07.2024.
 * Copyright (c) 2024-2024, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#include <Vortex/Core/Assertions.hpp>
#include <Vortex/Core/Core.hpp>
#include <Vortex/Core/Profiler.hpp>
#include <Vortex/Core/Timer.hpp>
#include <Vortex/Engine/Application.hpp>
#include <Vortex/Engine/RuntimeAssetManager.hpp>
#include <Vortex/Renderer/ImGui/ImGuiLayer.hpp>
#include <Vortex/Scene/Scene.hpp>
#include <Vortex/Scene/SceneRenderer.hpp>
#include <Vortex/Script/ScriptEngine.hpp>
#include <Vortex/Window/Input/Keyboard.hpp>
#include <Vortex/Window/Input/Mouse.hpp>

#include <cstdio>

import Vortex.Physics;
import Vortex.Renderer;

namespace Vortex
{
    Application* Application::s_Instance = nullptr;

    namespace
    {
        /// Printed once, when the first application starts (not for a
        /// program's headless commands, which never create one).
        void PrintBranding()
        {
            static bool printed = false;
            if (printed) return;
            printed = true;
            std::puts(
                R"( __      __        _              ______             _
 \ \    / /       | |            |  ____|           (_)
  \ \  / /__  _ __| |_ _____  __ | |__   _ __   __ _ _ _ __   ___
   \ \/ / _ \| '__| __/ _ \ \/ / |  __| | '_ \ / _` | | '_ \ / _ \
    \  / (_) | |  | ||  __/>  <  | |____| | | | (_| | | | | |  __/
     \/ \___/|_|   \__\___/_/\_\ |______|_| |_|\__, |_|_| |_|\___|
                                                __/ |
                                               |___/)");
            VtCoreTrace("Launching the {}...", g_EngineName);
        }
    } // namespace

    Application::Application(const ApplicationSpecification& specification)
    {
        PrintBranding();
        VtProfilerSetThreadName("Main Thread");
        VtCoreAssertMsg(s_Instance == nullptr,
                        "Only one instance of application might exist.");
        s_Instance = this;

        Initialize(specification);
    }
    Application::~Application()
    {
        Shutdown();
    }

    bool Application::Run()
    {
        m_Running = true;
        Timer fpsTimer;
        Timer deltaTimer;
        u64   frames = 0;

        do
        {
            if (fpsTimer.ElapsedTime().Seconds() > 1.0)
            {
                m_FpsCounter = frames;
                frames       = 0;
                fpsTimer.Restart();
            }

            {
                VtProfileScope("Poll events");
                Window::PollEvents();
            }
            // Finished background loads hand their assets over.
            m_AssetManager->Update();

            if (!m_MainWindow->IsMinimized())
            {
                {
                    VtProfileScope("Update");
                    for (auto layer : m_LayerStack) layer->OnUpdate(m_DeltaTime);
                    m_EventBus.DispatchDelayed();
                }

                // Layers record offscreen work (scene views) into the
                // frame's command list; ImGui then renders into the window
                // and presents.
                Renderer::BeginFrame();
                {
                    VtProfileScope("Render");
                    for (auto layer : m_LayerStack) layer->OnRender();
                }
                RenderImGui();
                Renderer::EndFrame();
                ++frames;
            }
            VtProfileFrameMark;

            if (!m_MainWindow->IsOpen()) break;
            m_DeltaTime = deltaTimer.Restart();
        } while (m_Running);

        for (auto layer : ::std::views::reverse(m_LayerStack))
        {
            layer->OnDetach();
            layer.Reset();
        }
        m_LayerStack.Clear();
        return m_ShouldRestart;
    }
    void Application::Close() { m_Running = false; }
    void Application::Restart()
    {
        m_ShouldRestart = true;
        Close();
    }

    ::Ref<AssetManager> Application::AssetManager() { return m_AssetManager; }

    void Application::Initialize(const ApplicationSpecification& specification)
    {
        VtProfileFunction();
        m_Name       = specification.Name;
        m_Arguments  = specification.Arguments;
        m_Subsystems = specification.Subsystems;

        Window::Initialize(&m_EventBus);

        Renderer::Specification rendererSpecs{};
        rendererSpecs.ApplicationName    = m_Name;
        rendererSpecs.ApplicationVersion = Vortex::g_Version;
        rendererSpecs.FramesInFlight     = 2;
        rendererSpecs.PreferredAdapter   = specification.PreferredAdapter;
        if (!specification.RendererAPI.Empty()
            && !RHI::ParseAPI(specification.RendererAPI, rendererSpecs.API))
            VtCoreWarn("Application: Unknown renderer API '{}', using {}",
                       specification.RendererAPI, RHI::APIName(rendererSpecs.API));
        bool rendererReady = Renderer::Initialize(rendererSpecs);
        VtCoreAssertMsg(rendererReady,
                        "Application: Failed to initialize the renderer");

        WindowSpecification specs = {};
        specs.Title               = m_Name;
        specs.VideoMode
            = {static_cast<i32>(specification.WindowWidth),
               static_cast<i32>(specification.WindowHeight)};
        specs.Decorated           = true;
        specs.Monitor             = Monitor::PrimaryMonitor();
        specs.AlwaysOnTop         = false;
        specs.Resizable           = true;
        specs.Fullscreen          = false;
        specs.Decorated           = true;
        specs.VSync               = specification.VSync;

        m_MainWindow              = CreateScope<RenderTargetWindow>(specs);
        m_ImGuiLayer = CreateRef<ImGuiLayer>("ImGuiLayer", *m_MainWindow,
                                             m_EventBus);
        PushOverlay(m_ImGuiLayer);

        Keyboard::Initialize(&m_EventBus);
        Mouse::Initialize(&m_EventBus);

        if (m_Subsystems.SceneRenderer)
        {
            bool sceneRendererReady = SceneRenderer::Initialize();
            VtCoreAssertMsg(
                sceneRendererReady,
                "Application: Failed to initialize the scene renderer");
        }

        if (m_Subsystems.Physics && !PhysicsEngine::Initialize())
            VtCoreErrorTag("Application", "Physics is unavailable");

#define BUILD_SCRIPT_ENGINE 1
#if BUILD_SCRIPT_ENGINE
        if (m_Subsystems.Scripting) ScriptEngine::Initialize();
#endif

        m_AssetManager = CreateRef<RuntimeAssetManager>();
    }
    void Application::Shutdown()
    {
        VtProfileFunction();
        m_AssetManager.Reset();
#if BUILD_SCRIPT_ENGINE
        if (m_Subsystems.Scripting) ScriptEngine::Shutdown();
#endif

        // GPU resources first (everything holding RHI objects), then the
        // device, then the windows.
        Renderer::WaitIdle();
        m_LayerStack.Clear();
        m_ImGuiLayer.Reset();
        // The active scene holds meshes, materials and textures.
        Scene::SwitchTo(nullptr);
        // After every scene (and its physics world) is gone.
        if (m_Subsystems.Physics) PhysicsEngine::Shutdown();
        if (m_Subsystems.SceneRenderer) SceneRenderer::Shutdown();
        m_MainWindow.Reset();

        Renderer::Shutdown();
        Window::Shutdown();
        s_Instance = nullptr;
    }

    void Application::RenderImGui()
    {
        m_ImGuiLayer->Begin();
        for (auto layer : m_LayerStack) layer->OnImGuiRender();
        m_ImGuiLayer->End();
    }
}; // namespace Vortex
