//
// Created by Vitriol1744 on 22.06.2021.
//
#pragma once

#include "Core.hpp"

namespace Vortex
{
    class VT_API Application : Singleton<Application>
    {
        public:
            Application();
            ~Application() = default;

            void Run();

            virtual void Update() { }
            virtual void Render() { }

        protected:
            bool running = true;
            int32 framerateLimit = 0; // 0 means that framerate limit is disabled.

        private:
            int32 frames = 0;
    };

    extern Application* CreateApplication();
}
