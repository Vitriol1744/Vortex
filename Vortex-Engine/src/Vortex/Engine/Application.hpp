//
// Created by Vitriol1744 on 08.09.2022.
//
#ifndef VORTEX_APPLICATION_HPP
#define VORTEX_APPLICATION_HPP

#include "Vortex/Core/Core.hpp"

#include "Vortex/Graphics/Window/IWindow.hpp"

namespace Vortex
{
    class VT_API Application
    {
        public:
            inline Application(std::string applicationName = "Vortex Application", Math::Vec3 applicationVersion = { 1, 0, 0 })
                    : applicationName(applicationName), applicationVersion(applicationVersion) { }
            virtual ~Application() = default;

            inline std::string_view GetName() const noexcept { return applicationName; }
            inline Math::Vec3 GetVersion() const noexcept { return applicationVersion; }
            inline Ref<IWindow>& GetWindow() noexcept { return window; }
            //inline Ref<IWindow>& GetWindow2() noexcept { return window2; }

            virtual void Initialize() { }
            virtual void Shutdown() { }

            virtual void Update() { }
            virtual void Render() { }

        protected:
            Ref<IWindow> window;
            Ref<IWindow> window2;

            std::string applicationName     = "Vortex Application";
            Math::Vec3  applicationVersion  = { 1, 0, 0 };

            std::vector<char*> arguments;
    };

    extern Application* CreateApplication(std::vector<char*> arguments);
}


#endif //VORTEX_APPLICATION_HPP
