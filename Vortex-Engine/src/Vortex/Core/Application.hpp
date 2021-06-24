//
// Created by Vitriol1744 on 22.06.2021.
//
#pragma once

#include "Core.hpp"
#include "WindowManager.hpp"

namespace Vortex
{
    class VT_API Application
    {
        public:
            Application();
            ~Application() = default;

            void Run();

        private:
            bool running = true;

            Ref<IWindow> window;
    };

    extern Application* CreateApplication();
}
