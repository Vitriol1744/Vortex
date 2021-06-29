//
// Created by Vitriol1744 on 22.06.2021.
//
#include "Sandbox.hpp"

#include "Vortex/Core/EntryPoint.hpp"
#include "Vortex/Core/Time.hpp"

#include <iostream>

Sandbox::Sandbox()
{
    VT_LOG_TRACE("Hello, World!");

    window = Vortex::WindowManager::Get()->NewWindow(800, 600, L"Vortex");

    Vortex::IRendererAPI::Get()->SetClearColor(Vortex::Vec4(0.0f, 1.0f, 1.0f, 1.0f));
}

void Sandbox::Update()
{
    running = window->IsOpen();
    window->Update();

    window->SetTitle(L"FPS: " + std::to_wstring(Vortex::Time::Get()->GetFPSCounter()));
}

void Sandbox::Render()
{
    Vortex::IRendererAPI::Get()->Clear();

    window->Present();
}

Vortex::Application* Vortex::CreateApplication()
{
    return new Sandbox;
}

