//
// Created by Vitriol1744 on 05.09.2022.
//
#ifndef VORTEX_ENGINE_HPP
#define VORTEX_ENGINE_HPP

#include "Vortex/Core/Core.hpp"

#include <vector>

namespace Vortex
{
    class VT_API Engine
    {
        public:
            Engine(std::vector<char*>& args);

            void Initialize();
            void Shutdown();

            void Run();
            inline void Stop() { instance->running = false; }

            /*
             * Sets framerate limit to specified value
             * @param framerate_limit limit of frames to be displayed, 0 means that there should be no framerate limit
             */
            inline static void FramerateLimit(uint32 framerate_limit) { instance->framerate_limit = framerate_limit; }

        private:
            static Engine* instance;

            std::vector<char*> args;
            bool running = false;
            uint32 framerate_limit = 0;

            void Update();
            void Render();
    };
}


#endif //VORTEX_ENGINE_HPP
