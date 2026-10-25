/*
 * Created by v1tr10l7 on 17.07.2024.
 * Copyright (c) 2024-2024, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#include <Vortex/Core/Assertions.hpp>
#include <Vortex/Core/Profiler.hpp>
#include <Vortex/Core/Timer.hpp>
#include <Vortex/Engine/Application.hpp>
#include <Vortex/Engine/RuntimeAssetManager.hpp>
#include <Vortex/Renderer/RHI/Vulkan/VulkanImGuiLayer.hpp>
#include <Vortex/Renderer/Renderer.hpp>
#include <Vortex/Renderer/Renderer2D.hpp>
#include <Vortex/Renderer/SceneRenderer.hpp>
#include <Vortex/Script/ScriptEngine.hpp>
#include <Vortex/Window/Input/Keyboard.hpp>
#include <Vortex/Window/Input/Mouse.hpp>

namespace Vortex
{
    Application* Application::s_Instance = nullptr;

    Application::Application(const ApplicationSpecification& specification)
    {
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

            Window::PollEvents();

            if (!m_MainWindow->IsMinimized())
            {
                for (auto layer : m_LayerStack) layer->OnUpdate(m_DeltaTime);
                m_EventBus.DispatchDelayed();

                auto swapChain = m_MainWindow->SwapChain();
                Renderer::BeginFrame(swapChain);

                for (auto layer : m_LayerStack) layer->OnRender();

                Renderer::BeginRenderPass(swapChain->Framebuffer().Promote());
                RenderImGui();
                Renderer::EndRenderPass();
                Renderer::EndFrame();
                ++frames;

                m_MainWindow->Present();
            }

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
        VtProfileFunction() m_Name = specification.Name;

        Window::Initialize(&m_EventBus);

        RHI::Specification rhiSpecs;
        rhiSpecs.EngineName         = Vortex::g_EngineName;
        rhiSpecs.EngineVersion      = Vortex::g_Version;
        rhiSpecs.ApplicationName    = m_Name;
        rhiSpecs.ApplicationVersion = Vortex::g_Version;

        rhiSpecs.API                = RHI::API::eVulkan;
        rhiSpecs.MaxFramesInFlight  = 2;

        Renderer::Initialize(rhiSpecs);

        WindowSpecification specs = {};
        specs.VideoMode           = {800, 600};
        specs.Decorated           = true;
        specs.Monitor             = Monitor::PrimaryMonitor();
        specs.AlwaysOnTop         = false;
        specs.Resizable           = true;
        specs.Fullscreen          = false;
        specs.Decorated           = true;

        m_MainWindow              = CreateScope<RenderTargetWindow>(specs);
        m_ImGuiLayer = CreateRef<VulkanImGuiLayer>("VulkanImGuiLayer",
                                                   *m_MainWindow, m_EventBus);
        PushOverlay(m_ImGuiLayer);

        Keyboard::Initialize(&m_EventBus);
        Mouse::Initialize(&m_EventBus);

        auto framebuffer = m_MainWindow->SwapChain()->Framebuffer().Promote();
        Renderer2D::Initialize(framebuffer);
        SceneRenderer::Initialize(framebuffer);

#define BUILD_SCRIPT_ENGINE 1
#if BUILD_SCRIPT_ENGINE
        ScriptEngine::Initialize();
#endif

        m_AssetManager = CreateRef<RuntimeAssetManager>();
    }
    void Application::Shutdown()
    {
        VtProfileFunction();
        m_AssetManager.Reset();
#if BUILD_SCRIPT_ENGINE
        ScriptEngine::Shutdown();
#endif

        SceneRenderer::Shutdown();
        Renderer2D::Shutdown();
        m_MainWindow.Reset();

        Renderer::Shutdown();
        Window::Shutdown();

        m_ImGuiLayer.Reset();
        s_Instance = nullptr;
        m_LayerStack.Clear();
    }

    void Application::RenderImGui()
    {
        m_ImGuiLayer->Begin();
        for (auto layer : m_LayerStack) layer->OnImGuiRender();
        m_ImGuiLayer->End();
    }
}; // namespace Vortex
