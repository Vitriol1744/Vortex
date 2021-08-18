//
// Created by Vitriol1744 on 10.08.2021.
//
#include "Engine.hpp"

#include "Vortex/Core/Time.hpp"
#include "Vortex/Core/EventSystem.hpp"
#include "Vortex/Audio/AudioManager.hpp"
//TODO: Remove later!
#include "Vortex/Graphics/API/OpenGL46/OpenGL.hpp"
#include "Vortex/Utility/AudioLoader.hpp"
#include "Vortex/Core/Input/Gamepad.hpp"

#include <imgui/backends/imgui_impl_opengl3.h>
    #include "Vortex/Graphics/API/IRendererAPI.hpp"


#include <iostream>

namespace Vortex
{
    using namespace Audio;

    Engine* Engine::instance = nullptr;

    Engine::Engine(std::vector<char*>& arguments)
    {
        VT_CORE_ASSERT_MSG(instance == nullptr, "Engine can only have 1 instance!");
        instance = this;
        this->arguments = arguments;
    }

    void Engine::Initialize()
    {
        //NOTE: Logger should always be initialized first!
        LoggingManager::Instance(); // Initialize Logger!
        Platform::Internal::Initialize();
        Time::Instance(); // Initialize Time!
        AudioManager::Initialize();
        VTCoreLogInfo("");
        window = WindowManager::Instance()->NewWindow(width, height, "Vortex", nullptr);
        app = CreateApplication(arguments);
        VTCoreLogInfo("Arguments: ");
        for (int i = 0; i < arguments.size(); i++) VTCoreLogInfo("Arg[{}]: {}", i, arguments[i]);
        VTCoreLogInfo("");

        ImGui::CreateContext();
        ImGui::StyleColorsDark();

        ImGuiIO& io = ImGui::GetIO();
        io.BackendFlags |= ImGuiBackendFlags_HasMouseCursors;
        io.BackendFlags |= ImGuiBackendFlags_HasSetMousePos;

        io.KeyMap[ImGuiKey_Tab] = 0;
        io.KeyMap[ImGuiKey_LeftArrow] = 1;
        io.KeyMap[ImGuiKey_RightArrow] = 2;
        io.KeyMap[ImGuiKey_UpArrow] = 3;
        io.KeyMap[ImGuiKey_DownArrow] = 4;
        io.KeyMap[ImGuiKey_PageUp] = 5;
        io.KeyMap[ImGuiKey_PageDown] = 6;
        io.KeyMap[ImGuiKey_Home] = 7;
        io.KeyMap[ImGuiKey_End] = 8;
        io.KeyMap[ImGuiKey_Insert] = 9;
        io.KeyMap[ImGuiKey_Delete] = 10;
        io.KeyMap[ImGuiKey_Backspace] = 11;
        io.KeyMap[ImGuiKey_Space] = 12;
        io.KeyMap[ImGuiKey_Enter] = 13;
        io.KeyMap[ImGuiKey_Escape] = 14;
        io.KeyMap[ImGuiKey_KeyPadEnter] = 15;
        io.KeyMap[ImGuiKey_A] = 16;
        io.KeyMap[ImGuiKey_C] = 17;
        io.KeyMap[ImGuiKey_V] = 18;
        io.KeyMap[ImGuiKey_X] = 19;
        io.KeyMap[ImGuiKey_Y] = 20;
        io.KeyMap[ImGuiKey_Z] = 21;

        ImGui_ImplOpenGL3_Init("#version 450");
    }
    void Engine::Shutdown()
    {
        AudioManager::Shutdown();
        Platform::Internal::Shutdown();
    }

    void Engine::Run()
    {
        double previousFrame = Time::GetTime().Seconds();
        double fpsTimer = previousFrame;

        //TODO: Remove Later!

        app->Initialize();
        #pragma clang diagnostic push
        #pragma ide diagnostic ignored "EndlessLoop"
        #pragma ide diagnostic ignored "ConstantConditionsOC"
        while (running)
        {
            //Gamepad::PollInput();
            if (Time::GetTime().Seconds() - fpsTimer >= 1.0f)
            {
                Time::SetFPSCounter(frames);
                frames = 0;
                fpsTimer = Time::GetTime().Seconds();
            }

            Time::SetDeltaTime(Time::GetTime().Seconds() - previousFrame);
            previousFrame = Time::GetTime().Seconds();

            Update();
            Render();

            if (framerateLimit > 0) Time::Sleep(Timestep(1.0 / framerateLimit - (Time::GetTime().Seconds() - previousFrame)));

            frames++;
            EventSystem::Instance()->PollEvents();
        }
        #pragma clang diagnostic pop
        app->Shutdown();
    }

    void Engine::Update()
    {
        Graphics::IRendererAPI::Clear();
        app->Update();

        ImGuiIO& io = ImGui::GetIO();
        io.DisplaySize = ImVec2(width, height);
        io.DeltaTime = (float)Time::DeltaTime();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui::NewFrame();

        bool active = true;
        ImGui::Begin("My First Tool", &active, ImGuiWindowFlags_MenuBar);
        if (ImGui::BeginMenuBar())
        {
            if (ImGui::BeginMenu("File"))
            {
                if (ImGui::MenuItem("Open..", "Ctrl+O")) { /* Do stuff */  }
                if (ImGui::MenuItem("Save", "Ctrl+S"))   { /* Do stuff */  }
                if (ImGui::MenuItem("Close", "Ctrl+W"))  { active = false; }
                ImGui::EndMenu();
            }
            ImGui::EndMenuBar();
        }

        // Edit a color (stored as ~4 floats)
        float color[]
        {
            0.0f,
            1.0f,
            1.0f,
            1.0f
        };
        ImGui::ColorEdit4("Color", color);

        // Plot some values
        const float my_values[] = { 0.2f, 0.1f, 1.0f, 0.5f, 0.9f, 2.2f };
        ImGui::PlotLines("Frame Times", my_values, IM_ARRAYSIZE(my_values));

        // Display contents in a scrolling region
        ImGui::TextColored(ImVec4(1,1,0,1), "Important Stuff");
        ImGui::BeginChild("Scrolling");
        for (int n = 0; n < 50; n++) ImGui::Text("%04d: Some text", n);
        ImGui::EndChild();
        ImGui::End();

        static bool show = true;
        ImGui::ShowDemoWindow(&show);

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }

    void Engine::Render()
    {
        app->Render();
    }
}
