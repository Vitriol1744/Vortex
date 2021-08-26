//
// Created by Vitriol1744 on 22.06.2021.
//
#include "Sandbox.hpp"

#include <cmath>
#include <iostream>
#include <Vortex/Graphics/Renderer.hpp>

#include "Vortex/Core/Input/Input.hpp"

using namespace Vortex::Math;
using namespace Vortex::Utility;

Vec2 lastMousePos;
bool Sandbox::OnMouseMove(IWindow* window, Vec2 pos)
{
    return false;
}

bool Sandbox::OnMouseScroll(IWindow* window, Vec2 scroll)
{
    return false;
}

Sandbox::Sandbox(std::vector<char*>& arguments)
{

}

void Sandbox::Initialize()
{
    lastMousePos = Input::Mouse::CursorPosition();
    width = 1280;
    height = 728;
    window = Engine::GetWindow();
    Renderer2D::Initialize();
    VTAddListener(WindowEvents::mouseMovedEvent, &Sandbox::OnMouseMove, this, VT_PH(1), VT_PH(2));
    VTAddListener(WindowEvents::mouseScrolledEvent, &Sandbox::OnMouseScroll, this, VT_PH(1), VT_PH(2));


    cameraController.GetCamera().CreatePerspective(75, width / height, 0.1f, 100.0f);
    cameraController.GetCamera().SetPosition({0.0f, 0.0f, 0.0f});

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

    #pragma region vertices
    float32 vertices[]
    {
        -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
         0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
        -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,

        -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
         0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
        -0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
        -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,

        -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
        -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
        -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
        -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
        -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
        -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,

         0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
         0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
         0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
         0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
         0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
         0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,

        -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
         0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
         0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
         0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,

        -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
        -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
        -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f
    };
    #pragma endregion
    
    #pragma region indices
    uint32 indices[]
    {
        //Top
        0, 1, 2,
        3, 4, 5,
        6, 7, 8,
        9, 10, 11,
        12, 13, 14,
        15, 16, 17,
        18, 19, 20,
        21, 22, 23,
        24, 25, 26,
        27, 28, 29,
        30, 31, 32,
        33, 34, 35

    };
    #pragma endregion

    vertexArray     = IVertexArray::Create();
    vertexArray->Bind();
    vertexBuffer    = IVertexBuffer::Create(vertices, sizeof(vertices));
    vertexBuffer->Bind();
    indexBuffer     = IIndexBuffer::Create(indices, sizeof(indices));
    indexBuffer->Bind();
    VertexBufferLayout layout;
    layout.AddElement(ShaderDataType::Float3);
    layout.AddElement(ShaderDataType::Float3);
    vertexBuffer->SetLayout(layout);
    vertexArray->AddVertexBuffer(vertexBuffer);
    vertexArray->SetIndexBuffer(indexBuffer);
    shader1 = IShader::Create("assets/shaders/basic.vert", "assets/shaders/basic.frag", false);
    shader2 = IShader::Create("assets/shaders/light.vert", "assets/shaders/light.frag", false);
}
void Sandbox::Shutdown()
{

}

void Sandbox::Update()
{
    if (!window->IsOpen()) Engine::Stop();
    IWindow::PollEvents();

    cameraController.Update();
    window->SetTitle("FPS: " + std::to_string(Time::FPSCounter()));

    if (Mouse::IsButtonPressed(MouseCode::Left)) window->SetCursor(CursorShape::Grabbed);
    if (Mouse::IsButtonPressed(MouseCode::Middle)) window->SetCursor(CursorShape::IBeam);
}
void Sandbox::Render()
{
    IRendererAPI::SetClearColor({ 0.1f, 0.1f, 0.1f, 1.0f });
    Renderer::BeginScene(cameraController.GetCamera());

    int radius = 5;
    Vec3 lightPos(radius * sin(Time::GetTime().Seconds()), 0.0f, radius * cos(Time::GetTime().Seconds()));
    Vec3 lightColor(1.0f, 1.0f, 0.2f);

    lightPos += Vec3(6.0f, 0.0f, -11.0f);

    shader1->Bind();
    shader1->SetUniform3f("u_LightPos", lightPos);
    shader1->SetUniform3f("u_ObjectColor", Vec3(0.4f, 0.6f, 0.2f));
    shader1->SetUniform3f("u_LightColor", lightColor);
    shader1->SetUniform3f("u_ViewPos", cameraController.GetCamera().GetPosition());
    shader1->SetUniform1f("u_Shininess", 128);
    
    Mat4 model = Translate(Mat4(1.0f), Vec3(6.0f, 0.0f, -11.0f));
    Renderer::DrawMesh(shader1, vertexArray, model);

    model = Translate(Mat4(1.0f), Vec3(8.0f, 0.0f, -10.0f));
    Renderer::DrawMesh(shader1, vertexArray, model);

    Vec3 cubePositions[] =
    {
        Vec3( 0.0f,  0.0f,  0.0f),
        Vec3( 2.0f,  5.0f, -15.0f),
        Vec3(-1.5f, -2.2f, -2.5f),
        Vec3(-3.8f, -2.0f, -12.3f),
        Vec3( 2.4f, -0.4f, -3.5f),
        Vec3(-1.7f,  3.0f, -7.5f),
        Vec3( 1.3f, -2.0f, -2.5f),
        Vec3( 1.5f,  2.0f, -2.5f),
        Vec3( 1.5f,  0.2f, -1.5f),
        Vec3(-1.3f,  1.0f, -1.5f)
    };

    for (int i = 0; i < 10; i++)
    {
        model = Translate(Mat4(1.0f), cubePositions[i]);
        float32 angle = 20.0f * i + sin(Time::GetTime().Seconds()) * 100;
        model = Rotate(model, angle, Vec3(1.0f, 0.3f, 0.5f));
        
        Renderer::DrawMesh(shader1, vertexArray, model);
    }

    shader2->Bind();
    shader2->SetUniform3f("u_LightColor", lightColor);
    
    model = Translate(Mat4(1.0f), lightPos);
    Renderer::DrawMesh(shader2, vertexArray, model);
    
    Renderer::EndScene();
    window->Present();
}

Vortex::Application* Vortex::CreateApplication(std::vector<char*> arguments)
{
    return new Sandbox(arguments);
}

