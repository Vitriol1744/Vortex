//
// Created by Vitriol1744 on 28.06.2021.
//
#pragma once

#include "Vortex/Core/Core.hpp"
#include "Vortex/Core/Timestep.hpp"

namespace Vortex
{
    class VT_API Time : public Singleton<Time>
    {
        public:
            friend class Engine;

            inline static Timestep GetTime() { return Timestep(Platform::GetCurrentTime()); }
            inline static void Sleep(Timestep timestep) { Platform::Sleep(timestep.Seconds()); }

            VT_NODISCARD inline static float64 DeltaTime() { return deltaTime; }
            VT_NODISCARD inline static int32 FPSCounter() { return fpsCounter; }

        private:
            static float64 deltaTime;
            static uint32 fpsCounter;

            inline static void SetDeltaTime(float64 deltaTime) { Time::deltaTime = deltaTime; }
            inline static void SetFPSCounter(int32 fpsCounter) { Time::fpsCounter = fpsCounter; }
    };

    template class VT_API Singleton<Time>;
}


