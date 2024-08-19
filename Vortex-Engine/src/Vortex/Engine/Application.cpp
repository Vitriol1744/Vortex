/*
 * Created by v1tr10l7 on 17.07.2024.
 * Copyright (c) 2024-2024, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#include "vtpch.hpp"

#include "Vortex/Core/Assertions.hpp"
#include "Vortex/Core/Events/EventSystem.hpp"
#include "Vortex/Engine/Application.hpp"
#include "Vortex/Renderer/Renderer.hpp"

namespace Vortex
{
    Application* Application::s_Instance = nullptr;

    Application::Application(const ApplicationSpecification& specification)
    {
        VtCoreAssertMsg(s_Instance == nullptr,
                        "Only one instance of application might exist.");
        s_Instance                = this;
        m_Name                    = specification.Name;

        WindowSpecification specs = {};
        specs.VideoMode           = {800, 600};
        specs.Decorated           = true;
        specs.Monitor             = Monitor::GetPrimaryMonitor();
        specs.AlwaysOnTop         = false;
        specs.Resizable           = true;
        specs.Fullscreen          = false;
        specs.Decorated           = true;

        m_MainWindow              = Window::Create(specs);
        Renderer::Initialize();
        m_ImGuiLayer = CreateRef<VulkanImGuiLayer>("VulkanImGuiLayer");
        PushOverlay(m_ImGuiLayer);
    }
    Application::~Application()
    {
        Renderer::Shutdown();
        s_Instance = nullptr;
    }

    bool Application::Run()
    {
        m_Running = true;
        do {
            Window::PollEvents();
            for (auto layer : m_LayerStack) layer->OnUpdate();

            Renderer::BeginFrame(m_MainWindow);

            for (auto layer : m_LayerStack) layer->OnRender();

            m_ImGuiLayer->Begin();
            for (auto layer : m_LayerStack) layer->OnImGuiRender();
            m_ImGuiLayer->End();

            Renderer::EndFrame();

            m_MainWindow->Present();
            EventSystem::PollEvents();
            m_Running = m_MainWindow->IsOpen();
        } while (m_Running);

        for (auto layer : std::views::reverse(m_LayerStack)) layer->OnDetach();
        return m_ShouldRestart;
    }
    void Application::Close() { m_Running = false; }
}; // namespace Vortex
