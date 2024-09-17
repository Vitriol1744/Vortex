/*
 * Created by v1tr10l7 on 17.07.2024.
 * Copyright (c) 2024-2024, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#include "vtpch.hpp"

#include "Vortex/Core/Assertions.hpp"
#include "Vortex/Core/Events/EventSystem.hpp"
#include "Vortex/Core/Profiler.hpp"
#include "Vortex/Core/Timer.hpp"
#include "Vortex/Engine/Application.hpp"
#include "Vortex/Renderer/Renderer.hpp"
#include "Vortex/Renderer/Renderer2D.hpp"
#include "Vortex/Window/Input/Keyboard.hpp"

namespace Vortex
{
    Application* Application::s_Instance = nullptr;

    Application::Application(const ApplicationSpecification& specification)
    {
        VtProfilerSetThreadName("Main Thread");
        VtCoreAssertMsg(s_Instance == nullptr,
                        "Only one instance of application might exist.");
        s_Instance = this;
        m_Name     = specification.Name;

        Window::Initialize();
        Renderer::Initialize();

        WindowSpecification specs = {};
        specs.VideoMode           = {800, 600};
        specs.Decorated           = true;
        specs.Monitor             = Monitor::GetPrimaryMonitor();
        specs.AlwaysOnTop         = false;
        specs.Resizable           = true;
        specs.Fullscreen          = false;
        specs.Decorated           = true;

        m_MainWindow              = Window::Create(specs);
        m_ImGuiLayer = CreateRef<VulkanImGuiLayer>("VulkanImGuiLayer");
        PushOverlay(m_ImGuiLayer);

        Keyboard::Initialize();
        Renderer2D::Initialize();
    }
    Application::~Application()
    {
        Renderer2D::Shutdown();
        m_MainWindow.reset();

        Renderer::Shutdown();
        Window::Shutdown();

        s_Instance = nullptr;
    }

    bool Application::Run()
    {
        m_Running = true;
        Timer fpsTimer;
        Timer deltaTimer;
        u64   frames = 0;

        do {
            if (fpsTimer.ElapsedTime().Seconds() > 1.0)
            {
                m_FpsCounter = frames;
                frames       = 0;
                fpsTimer.Restart();
            }

            Window::PollEvents();

            if (!m_MainWindow->IsMinimized())
            {
                for (auto layer : m_LayerStack) layer->OnUpdate();

                Renderer::BeginFrame(*m_MainWindow);

                for (auto layer : m_LayerStack) layer->OnRender();

                Renderer::BeginRenderPass(
                    m_MainWindow->GetSwapChain()->GetFramebuffer().lock());
                RenderImGui();
                Renderer::EndRenderPass();
                Renderer::EndFrame();
                ++frames;

                m_MainWindow->Present();
            }
            EventSystem::PollEvents();
            if (!m_MainWindow->IsOpen()) break;
            m_DeltaTime = deltaTimer.Restart();
        } while (m_Running);

        for (auto layer : std::views::reverse(m_LayerStack))
        {
            layer->OnDetach();
            layer.reset();
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

    void Application::RenderImGui()
    {
        m_ImGuiLayer->Begin();
        for (auto layer : m_LayerStack) layer->OnImGuiRender();
        m_ImGuiLayer->End();
    }
}; // namespace Vortex
