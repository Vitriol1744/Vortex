//
// Created by Vitriol1744 on 10.08.2021.
//
#pragma once

#include "Vortex/Core/Core.hpp"

#include "Vortex/Engine/Application.hpp"

#include "Vortex/Graphics/ShaderLibrary.hpp"

#include "Vortex/Window/WindowManager.hpp"

#include <vector>

namespace Vortex
{
    class VT_API Engine
    {
        public:
            Engine(std::vector<char*>& arguments);

            void Initialize();
            void Shutdown();
            
            void Run();
            inline static void Stop() { instance->running = false; }

            inline static Ref<IWindow> GetWindow() { return instance->window; }
            inline static Graphics::ShaderLibrary& GetShadersLibrary() { return instance->shadersLibrary; }

            inline static void SetFramerateLimit(int32 framerateLimit) { instance->framerateLimit = framerateLimit; }

        private:
            static Engine* instance;
            Ref<IWindow> window;
            Application* app;
            Graphics::ShaderLibrary shadersLibrary;

            bool running = true;
            int32 framerateLimit = 0; // 0 means that framerate limit is disabled.
            int32 frames = 0;

            int32 width = 1280;
            int32 height = 720;
            
            std::vector<char*> arguments;

            void Update();
            void Render();
    };
}