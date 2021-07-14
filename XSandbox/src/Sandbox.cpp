//
// Created by Vitriol1744 on 22.06.2021.
//
#include "Sandbox.hpp"

#include "Vortex/Core/EntryPoint.hpp"
#include "Vortex/Core/Time.hpp"
#include "Vortex/Graphics/IVertexBuffer.hpp"
#include "Vortex/Graphics/IIndexBuffer.hpp"
#include "Vortex/Graphics/IMesh.hpp"

#include <iostream>

void Sandbox::OnMouseMove(Math::Vec2 pos)
{
    pos.x = pos.x * 16 / 960.0f;
    pos.y = 9.0f - pos.y * 9.0f / 540.0f;
    shader->SetUniform2f("lightPos", Math::Vec2(pos.x + 1.0, -pos.y + 7.0));
    VT_LOG_TRACE("MousePos: x: {}, y: {}", pos.x, pos.y);
}

Sandbox::Sandbox()
{
    VT_LOG_TRACE("Hello, World!");

    window = Vortex::WindowManager::Get()->NewWindow(960, 540, L"Vortex");

    window->HideCursor();

    window->mouseMovedEvent.AddListener("Sandbox::OnMouseMove", std::bind(&Sandbox::OnMouseMove, this, std::placeholders::_1));

    float32 vertices[]
    {
         0.0f,  0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
         0.0f,  3.0f, 0.0f, 0.0f, 1.0f, 0.0f,
         8.0f,  3.0f, 0.0f, 0.0f, 0.0f, 1.0f,
         8.0f,  0.0f, 0.0f, 0.0f, 1.0f, 0.0f
    };

    projection = Math::Mat4::Orthographic(0.0f, 16.0f, 0.0f, 9.0f, -1.0f, 1.0f);
    model = Math::Mat4::Translate(Math::Vec3(4, 3, 0));

    uint32 indices[]
    {
        0, 1, 2,
        2, 3, 0
    };

    mesh = VTGraphics::IMesh::Create();
    VTGraphics::VertexBufferLayout layout;
    layout.AddElement(VTGraphics::ShaderDataType::Float3);
    layout.AddElement(VTGraphics::ShaderDataType::Float3);
    mesh->Bind();

    vertexBuffer = VTGraphics::IVertexBuffer::Create(vertices, sizeof(vertices));
    vertexBuffer->SetLayout(layout);
    indexBuffer = VTGraphics::IIndexBuffer::Create(indices, sizeof(indices));
    mesh->AddVertexBuffer(vertexBuffer);
    mesh->SetIndexBuffer(indexBuffer);
    shader = VTGraphics::IShader::Create("assets/shaders/basic.vert", "assets/shaders/basic.frag", false);
    shader->Bind();

    shader->SetUniform2f("lightPos", Math::Vec2(4.5f, 1.5f));
    shader->SetUniform4f("uColor", Math::Vec4(0.2f, 0.3f, 0.8f, 1.0f));
    shader->SetMat4f("uProjection", projection);
    shader->SetMat4f("uModel", Math::Mat4::Translate(Math::Vec3(4, 3, 0)));
}

void Sandbox::Update()
{
    running = window->IsOpen();
    window->Update();

    window->SetTitle(L"FPS: " + std::to_wstring(Vortex::Time::Get()->GetFPSCounter()));
}

void Sandbox::Render()
{
    y += 0.01;
    y2 -= 0.01;

    if (y > 1) y = 0;
    if (y2 < 0) y2 = 1;
    Vortex::WindowManager::Get()->Bind(window);
    VTGraphics::IRendererAPI::Get()->SetClearColor(VTMath::Vec4(0.0f, y, 1.0f, 1.0f));
    VTGraphics::IRendererAPI::Get()->Clear();

    mesh->Bind();
    vertexBuffer->Bind();
    indexBuffer->Bind();
    VTGraphics::IRendererAPI::Get()->DrawIndexed(mesh, 6);

    window->Present();
}

Vortex::Application* Vortex::CreateApplication()
{
    return new Sandbox;
}

