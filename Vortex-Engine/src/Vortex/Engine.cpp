//
// Created by Vitriol1744 on 05.09.2022.
//
#include "Engine.hpp"

namespace Vortex
{
    Engine* Engine::instance = nullptr;

    Engine::Engine(std::vector<char*>& args)
    {
        //VT_CORE_ASSERT_MSG(instance == nullptr, "Engine can only have one instance!");
        instance = this;
        this->args = args;
    }

    void Engine::Initialize()
    {
        //NOTE: LoggingManager should always be initialized first, because every other component/subsystem is using it
        LoggingManager::GetInstance(); // Initialize Logging Manager
    }
    void Engine::Shutdown()
    {

    }

    void Engine::Run()
    {
        running = true;
        while (running)
        {
            Update();
            Render();
        }
    }

    void Engine::Update()
    {

    }
    void Engine::Render()
    {

    }
}