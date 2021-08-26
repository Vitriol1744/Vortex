//
// Created by Vitriol1744 on 22.06.2021.
//
#pragma once

#include "Vortex/Core/Core.hpp"

#include "Vortex/Engine/Engine.hpp"

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

            inline void PushOverlay(Graphics::Layer* overlay) { Engine::GetLayerStack().PushOverlay(overlay); }
            inline void PushLayer(Graphics::Layer* layer) { Engine::GetLayerStack().PushLayer(layer); }

            inline void PopOverlay(Graphics::Layer* overlay) { Engine::GetLayerStack().PopOverlay(overlay); }
            inline void PopLayer(Graphics::Layer* layer) { Engine::GetLayerStack().PopLayer(layer); }

        protected:
            std::vector<char*> arguments;
    };

    extern Application* CreateApplication(std::vector<char*> arguments);
}
