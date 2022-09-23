//
// Created by Vitriol1744 on 05.09.2022.
//
#ifndef VORTEX_ENGINE_HPP
#define VORTEX_ENGINE_HPP

#include "Vortex/Core/Core.hpp"
#include "Vortex/Engine/Application.hpp"
#include "Vortex/Graphics/Window/WindowManager.hpp"

#include "Vortex/Graphics/API/IVertexBuffer.hpp"
#include "Vortex/Graphics/API/IIndexBuffer.hpp"

namespace Vortex
{
    class VT_API Engine
    {
        public:
            Engine(std::vector<char*>& args);

            bool Initialize();
            void Shutdown();

            void Run();
            inline void Stop() { instance->running = false; }

            inline static Ref<IWindow> GetWindow() { return instance->app->GetWindow(); }
            inline static Application* GetApplication() { return instance->app; }

            /*
             * Sets framerate limit to specified value
             * @param framerate_limit limit of frames to be displayed, 0 means that there should be no framerate limit
             */
            inline static void SetFramerateLimit(uint32 framerate_limit) { instance->framerate_limit = framerate_limit; }

        private:
            static Engine* instance;
            Application* app;
            Ref<Graphics::IVertexBuffer> vertexBuffer;
            Ref<Graphics::IIndexBuffer> indexBuffer;

            std::vector<char*> args;
            bool running = false;
            uint32 framerate_limit = 0;

            uint32 width = 1280;
            uint32 height = 720;

            void Update();
            void Render();
    };
}


#endif //VORTEX_ENGINE_HPP
