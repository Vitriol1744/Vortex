/*
 * Created by v1tr10l7 on 17.07.2024.
 * Copyright (c) 2024-2024, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#include "vtpch.hpp"

#include "Vortex/Core/Assertions.hpp"
#include "Vortex/Engine/Application.hpp"

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
        specs.VideoMode           = {800, 600, 32};
        specs.Decorated           = true;

        m_MainWindow              = Window::Create(specs);
    }
    Application::~Application() { s_Instance = nullptr; }

    bool Application::Run()
    {
        m_Running = true;
        while (m_Running)
        {
            m_MainWindow->Present();
            Window::PollEvents();
            m_Running = m_MainWindow->IsOpen();
        }
        return m_ShouldRestart;
    }
    void Application::Close() { m_Running = false; }
}; // namespace Vortex
