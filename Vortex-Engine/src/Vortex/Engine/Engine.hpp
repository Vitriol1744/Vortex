//
// Created by Vitriol1744 on 10.08.2021.
//
#pragma once

#include "Vortex/Core/Core.hpp"

#include "Vortex/Engine/Application.hpp"

#include "Vortex/Graphics/ShaderLibrary.hpp"

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

            inline static Graphics::ShaderLibrary& GetShadersLibrary() { return instance->shadersLibrary; }

            inline static void SetFramerateLimit(int32 framerateLimit) { instance->framerateLimit = framerateLimit; }

        private:
            static Engine* instance;
            Application* app;
            Graphics::ShaderLibrary shadersLibrary;

            bool running = true;
            int32 framerateLimit = 0; // 0 means that framerate limit is disabled.
            int32 frames = 0;
            
            std::vector<char*> arguments;
    };
}