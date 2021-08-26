//
// Created by Vitriol1744 on 10.08.2021.
//
#include "Engine.hpp"

#include "Vortex/Audio/AudioManager.hpp"

#include "Vortex/Core/EventSystem.hpp"
#include "Vortex/Core/Input/Gamepad.hpp"
#include "Vortex/Core/Time.hpp"

#include "Vortex/Engine/Application.hpp"

#include "Vortex/Graphics/API/IRendererAPI.hpp"

//TODO: Remove later!
#include "Vortex/Graphics/ImGui/Backend_ImGui_OpenGL3.hpp"
#include "Vortex/Graphics/ImGui/ImGui.hpp"

#include "Vortex/Utility/AudioLoader.hpp"

namespace Vortex
{
    using namespace Audio;
    using namespace Graphics;

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
        IRendererAPI::Initialize();
        VTCoreLogInfo("");
        window = WindowManager::Instance()->NewWindow(width, height, "Vortex", nullptr);
        app = CreateApplication(arguments);
        VTCoreLogInfo("Arguments: ");
        for (int i = 0; i < arguments.size(); i++) VTCoreLogInfo("Arg[{}]: {}", i, arguments[i]);
        VTCoreLogInfo("");

        Graphics::ImGuiRenderer::Initialize(window);
    }
    void Engine::Shutdown()
    {
        delete app;
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
        using namespace Graphics;
        IRendererAPI::Clear();
        app->Update();

        ImGuiRenderer::Begin();
        
        static bool show = true;
        ImGui::ShowDemoWindow(&show);
        
        ImGuiRenderer::End();
    }

    void Engine::Render()
    {
        window->ActivateContext();
        app->Render();
    }
}
