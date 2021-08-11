//
// Created by Vitriol1744 on 22.06.2021.
//
#include "Sandbox.hpp"

#include <iostream>

using namespace Vortex::Utility;

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

Sandbox::Sandbox(std::vector<char*>& arguments)
{
    
}

void Sandbox::Initialize()
{
    window = WindowManager::Instance()->NewWindow(width, height, "Vortex");
    Renderer2D::Initialize();
    //window->SetIcon("assets/textures/");
    //window->SetPosition(333, 111);
    window->mouseMovedEvent.AddListener("Sandbox::OnMouseMove", std::bind(&Sandbox::OnMouseMove, this, std::placeholders::_1));

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

    #pragma region vertices
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
    #pragma endregion

    projection = Math::Mat4::Orthographic(0.0f, 16.0f, 0.0f, 9.0f, -1.0f, 1.0f);

    #pragma region indices
    uint32 indices[]
    {
        0, 1, 2, 3, 4, 5, 6,
        7, 8, 9, 10, 11, 12,
        13, 14, 15, 16, 17, 18,
        19, 20, 21, 22, 23, 24,
        25, 26, 27, 28, 29, 30,
        31, 32, 33, 34, 35, 36,
        };
    #pragma endregion

    shader1 = Graphics::IShader::Create("assets/shaders/basic.vert", "assets/shaders/basic.frag", false);
    shader1->Bind();
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
        sprite.Move(0, 1 * Time::DeltaTime());
        spritesCount++;
    }
    if(window->IsKeyPressed(Input::KeyCode::S))
    {
        sprite.Move(0, -1 * Time::DeltaTime());
        spritesCount--;
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
    Graphics::IRendererAPI::SetClearColor(Math::Vec4(0.3f, 0.3f, 0.3f, 1.0f));
    Graphics::IRendererAPI::Clear();

    //spritesCount = 15;
    //float xo = -2.5f;
    //float yo = -2.5f;
    //for (int y = 0; y < spritesCount; y++)
    //{
    //    float yp = y * 0.05f + yo;
    //    for (int x = 0; x < spritesCount; x++)
    //    {
    //        float xp = x * 0.05f + xo;
    //        Graphics::Renderer2D::DrawQuad({xp, yp}, { 0.045f, 0.045f }, Math::Vec4(x / spritesCount, 0.3f, y / spritesCount, 1.0f));
    //    }
    //}
    texture->Bind();
    shader1->SetUniform1i("u_Texture", 0);
    view = Math::Mat4(1.0f) * Math::Mat4::LookAt(Math::Vec3(0.0f, 0.0f, 3.0f), Math::Vec3(0.0f, 0.0f, -1.0f), Math::Vec3(0.0f, 1.0f, 1.0f));
    projection = Math::Mat4::Perspective(45.0f, (float32)width / (float32)height, 0.1f, 100.0f);
    shader1->SetUniformMat4f("uView", view);
    shader1->SetUniformMat4f("uProjection", projection);


    Graphics::Renderer2D::DrawSprite(sprite);
    Graphics::Renderer2D::Submit();

    window->Present();
}

Vortex::Application* Vortex::CreateApplication(std::vector<char*> arguments)
{
    return new Sandbox(arguments);
}

