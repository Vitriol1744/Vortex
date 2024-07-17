/*
 * Created by v1tr10l7 on 17.07.2024.
 * Copyright (c) 2024-2024, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#include "Application.hpp"

#include "Vortex/Core/Assertions.hpp"

#include <iostream>
#include <stdexcept>

namespace Vortex
{
    Application* Application::s_Instance = nullptr;

    Application::Application(const ApplicationSpecification& specification)
    {
        VtCoreAssertMsg(s_Instance == nullptr,
                        "Only one instance of application might exist.");
        s_Instance = this;
        m_Name     = specification.Name;
    }
    Application::~Application() { s_Instance = nullptr; }

    bool Application::Run()
    {
        while (m_Running)
            ;
        return m_ShouldRestart;
    }
    void Application::Close() { m_Running = false; }
}; // namespace Vortex
