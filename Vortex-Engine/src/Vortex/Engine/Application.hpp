//
// Created by Vitriol1744 on 22.06.2021.
//
#pragma once

#include "Vortex/Core/Core.hpp"

#include <vector>

namespace Vortex
{
    class VT_API Application
    {
        public:
            Application() = default;
            ~Application() = default;

            virtual void Initialize() { }
            virtual void Shutdown() { }

            virtual void Update() { }
            virtual void Render() { }

        protected:
            std::vector<char*> arguments;
    };

    extern Application* CreateApplication(std::vector<char*> arguments);
}
