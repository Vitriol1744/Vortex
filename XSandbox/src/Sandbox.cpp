//
// Created by Vitriol1744 on 22.06.2021.
//
#include "Sandbox.hpp"

#include <cmath>
#include <iostream>

using namespace Vortex::Utility;

void Sandbox::OnMouseMove(Math::Vec2 pos)
{

}

void Sandbox::OnMouseScroll(Math::Vec2 scroll)
{
    //cameraScale += scroll.x;
}

Sandbox::Sandbox(std::vector<char*>& arguments)
{

}

void Sandbox::Initialize()
{
    width = 1024;
    height = 600;
    window = WindowManager::Instance()->NewWindow(width, height, "Vortex");
    Renderer2D::Initialize();
    window->mouseMovedEvent.AddListener("Sandbox::OnMouseMove", std::bind(&Sandbox::OnMouseMove, this, std::placeholders::_1));
    window->mouseScrolledEvent.AddListener("Sandbox::OnMouseScroll", std::bind(&Sandbox::OnMouseScroll, this, std::placeholders::_1));


    camera.CreateOrthographic(-1.0f, 1.0f, -1.0f, 1.0f, -1.0f, 1.0f);
    camera.SetPosition({0.0f, 0.0f, 0.0f});

    buffer.Initialize();
    buffer.LoadFromFile("assets/sounds/breakout.wav");
    source.Initialize();
    
    source.SetPitch(1.0f);
    source.SetGain(1.0f);
    source.SetPosition(0.0f, 0.0f, 0.0f);
    source.SetVelocity(0.0f, 0.0f, 0.0f);
    source.SetLooping(false);
    source.SetBuffer(buffer);
    source.Play();

    texture = ITexture2D::Create("assets/textures/bmpTexture.bmp");
}
void Sandbox::Shutdown()
{

}

void Sandbox::Update()
{
    if (!window->IsOpen()) Engine::Stop();

    window->Update();

    if (window->IsKeyPressed(Input::KeyCode::W))
    {
        cameraPosition.y += Time::DeltaTime();
    }
    if(window->IsKeyPressed(Input::KeyCode::S))
    {
        cameraPosition.y -= Time::DeltaTime();
    }
    if(window->IsKeyPressed(Input::KeyCode::A))
    {
        cameraPosition.x -= Time::DeltaTime();
    }
    if(window->IsKeyPressed(Input::KeyCode::D))
    {
        cameraPosition.x += Time::DeltaTime();
    }

    camera.SetPosition(cameraPosition);
    window->SetTitle("FPS: " + std::to_string(Time::FPSCounter()));
}
void Sandbox::Render()
{
    Renderer2D::BeginScene(camera);
    IRendererAPI::SetClearColor(Math::Vec4(0.3f, 0.3f, 0.3f, 1.0f));
    IRendererAPI::Clear();
    camera.SetScale({1.5f, 1.5f, 1.5f});
    spritesCount = 15;
    float32 xo = -2.5f;
    float32 yo = -2.5f;
    for (int32 y = 0; y < spritesCount; y++)
    {
        float32 yp = y * 0.05f + yo;
        for (int32 x = 0; x < spritesCount; x++)
        {
            float32 xp = x * 0.05f + xo;
            Graphics::Renderer2D::DrawQuad({xp, yp}, { 0.045f, 0.045f }, glm::vec4((float32)x / spritesCount, 0.3f, (float32)y / spritesCount, 1.0f));
        }
    }
    
    texture->Bind();

    Renderer2D::DrawSprite(sprite);
    Renderer2D::Submit();

    Renderer2D::EndScene();
    window->Present();
}

Vortex::Application* Vortex::CreateApplication(std::vector<char*> arguments)
{
    return new Sandbox(arguments);
}

