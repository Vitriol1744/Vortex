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
            friend class Engine;

            inline Application(std::string applicationName = "Vortex Application", Math::Vec3 applicationVersion = { 1, 0, 0 })
                : applicationName(applicationName), applicationVersion(applicationVersion) { }
            ~Application() = default;

            inline std::string_view GetName() const noexcept { return applicationName; }
            inline Math::Vec3 GetVersion() const noexcept { return applicationVersion; }
            inline Ref<IWindow> GetWindow() const noexcept { return window; }

            virtual void Initialize() { }
            virtual void Shutdown() { }

            virtual void Update() { }
            virtual void Render() { }

            inline void PushOverlay(Graphics::Layer* overlay) { Engine::GetLayerStack().PushOverlay(overlay); }
            inline void PushLayer(Graphics::Layer* layer) { Engine::GetLayerStack().PushLayer(layer); }

            inline void PopOverlay(Graphics::Layer* overlay) { Engine::GetLayerStack().PopOverlay(overlay); }
            inline void PopLayer(Graphics::Layer* layer) { Engine::GetLayerStack().PopLayer(layer); }

        protected:
            Ref<IWindow> window;

            std::string applicationName     = "Vortex Application";
            Math::Vec3  applicationVersion  = { 1, 0, 0 };

            std::vector<char*> arguments;

        private:
            void CreateWindow(int32 width, int32 height, std::string_view title)
            {
                window = WindowManager::Instance()->NewWindow(800, 600, "Vortex", nullptr);
            }
    };

    extern Application* CreateApplication(std::vector<char*> arguments);
}
