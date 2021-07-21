//
// Created by Vitriol1744 on 22.06.2021.
//
#include "Sandbox.hpp"

#include "Vortex/Core/EntryPoint.hpp"
#include "Vortex/Core/Time.hpp"
#include "Vortex/Graphics/API/IVertexBuffer.hpp"
#include "Vortex/Graphics/API/IIndexBuffer.hpp"
#include "Vortex/Graphics/API/IVertexArray.hpp"

#include <iostream>

void Sandbox::OnMouseMove(Math::Vec2 pos)
{

}

struct Vertex
{
    Math::Vec3 position;
    Math::Vec4 color;
    Math::Vec2 texCoords;
    float32 textureIndex;
};

Sandbox::Sandbox()
{
    VT_LOG_TRACE("Hello, World!");

    window = Vortex::WindowManager::Instance()->NewWindow(width, height, L"Vortex");

    VTGraphics::Renderer2D::Initialize();

    window->HideCursor();

    window->mouseMovedEvent.AddListener("Sandbox::OnMouseMove", std::bind(&Sandbox::OnMouseMove, this, std::placeholders::_1));

    framerateLimit = 60;

    float32 vertices[]
    {
        -0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 0.0f,
         0.5f, -0.5f, -0.5f, 0.0f, 1.0f, 0.0f,
         0.5f,  0.5f, -0.5f, 0.0f, 0.0f, 1.0f,
         0.5f,  0.5f, -0.5f, 0.0f, 0.0f, 1.0f,
        -0.5f,  0.5f, -0.5f, 0.0f, 1.0f, 0.0f,
        -0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 0.0f,

        -0.5f, -0.5f,  0.5f, 1.0f, 0.0f, 0.0f,
         0.5f, -0.5f,  0.5f, 0.0f, 1.0f, 0.0f,
         0.5f,  0.5f,  0.5f, 0.0f, 0.0f, 1.0f,
         0.5f,  0.5f,  0.5f, 0.0f, 0.0f, 1.0f,
        -0.5f,  0.5f,  0.5f, 0.0f, 1.0f, 0.0f,
        -0.5f, -0.5f,  0.5f, 1.0f, 0.0f, 0.0f,

        -0.5f,  0.5f,  0.5f, 1.0f, 0.0f, 0.0f,
        -0.5f,  0.5f, -0.5f, 0.0f, 1.0f, 0.0f,
        -0.5f, -0.5f, -0.5f, 0.0f, 0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f, 0.0f, 0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f, 0.0f, 1.0f, 0.0f,
        -0.5f,  0.5f,  0.5f, 1.0f, 0.0f, 0.0f,

         0.5f,  0.5f,  0.5f, 1.0f, 0.0f, 0.0f,
         0.5f,  0.5f, -0.5f, 0.0f, 1.0f, 0.0f,
         0.5f, -0.5f, -0.5f, 0.0f, 0.0f, 1.0f,
         0.5f, -0.5f, -0.5f, 0.0f, 0.0f, 1.0f,
         0.5f, -0.5f,  0.5f, 0.0f, 1.0f, 0.0f,
         0.5f,  0.5f,  0.5f, 1.0f, 0.0f, 0.0f,

        -0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 0.0f,
         0.5f, -0.5f, -0.5f, 0.0f, 1.0f, 0.0f,
         0.5f, -0.5f,  0.5f, 0.0f, 0.0f, 1.0f,
         0.5f, -0.5f,  0.5f, 0.0f, 0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f, 0.0f, 1.0f, 0.0f,
        -0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 0.0f,

        -0.5f,  0.5f, -0.5f, 1.0f, 0.0f, 0.0f,
         0.5f,  0.5f, -0.5f, 0.0f, 1.0f, 0.0f,
         0.5f,  0.5f,  0.5f, 0.0f, 0.0f, 1.0f,
         0.5f,  0.5f,  0.5f, 0.0f, 0.0f, 1.0f,
        -0.5f,  0.5f,  0.5f, 0.0f, 1.0f, 0.0f,
        -0.5f,  0.5f, -0.5f, 1.0f, 0.0f, 0.0f,
    };

    projection = Math::Mat4::Orthographic(0.0f, 16.0f, 0.0f, 9.0f, -1.0f, 1.0f);


    uint32 indices[]
    {
        0, 1, 2, 3, 4, 5, 6,
        7, 8, 9, 10, 11, 12,
        13, 14, 15, 16, 17, 18,
        19, 20, 21, 22, 23, 24,
        25, 26, 27, 28, 29, 30,
        31, 32, 33, 34, 35, 36,
    };

    shader1 = VTGraphics::IShader::Create("assets/shaders/basic.vert", "assets/shaders/basic.frag", false);
    shader1->Bind();
}

void Sandbox::Update()
{
    running = window->IsOpen();
    window->Update();

    if (window->IsKeyPressed(Input::KeyCode::W))
    {
        sprite.Move(0, 1 * Time::DeltaTime());
    }
    if(window->IsKeyPressed(Input::KeyCode::S))
    {
        sprite.Move(0, -1 * Time::DeltaTime());
    }
    if(window->IsKeyPressed(Input::KeyCode::A))
    {
        sprite.Move(-1 * Time::DeltaTime(), 0);
    }
    if(window->IsKeyPressed(Input::KeyCode::D))
    {
        sprite.Move(1 * Time::DeltaTime(), 0);
    }

    window->SetTitle("FPS: " + std::to_string(Time::FPSCounter()));
}

void Sandbox::Render()
{
    shader1->Bind();
    VTGraphics::IRendererAPI::SetClearColor(Math::Vec4(0.0f, 1.0f, 1.0f, 1.0f));
    VTGraphics::IRendererAPI::Clear();

    VTGraphics::Renderer2D::DrawSprite(sprite);

    VTGraphics::Renderer2D::Submit();

    view = Math::Mat4(1.0f) * Math::Mat4::LookAt(Math::Vec3(0.0f, 0.0f, 3.0f), Math::Vec3(0.0f, 0.0f, -1.0f), Math::Vec3(0.0f, 1.0f, 1.0f));
    projection = Math::Mat4::Perspective(45.0f, (float32)width / (float32)height, 0.1f, 100.0f);
    shader1->SetUniformMat4f("uView", view);
    shader1->SetUniformMat4f("uProjection", projection);

    window->Present();
}

Vortex::Application* Vortex::CreateApplication()
{
    return new Sandbox;
}

