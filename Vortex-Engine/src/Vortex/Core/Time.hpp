//
// Created by Vitriol1744 on 28.06.2021.
//
#pragma once

#include "Core/Core.hpp"

namespace Vortex
{
    class VT_API Time : public Singleton<Time>
    {
        public:
            friend class Application;

            Time();
            static float64 GetTimeInSeconds();

            VT_NODISCARD inline float64 GetDeltaTime() const { return deltaTime; }
            VT_NODISCARD inline int32 GetFPSCounter() const { return fpsCounter; }

        private:
            float64 deltaTime = 0.0f;
            int32 fpsCounter = 0;

            inline void SetDeltaTime(float64 deltaTime) { this->deltaTime = deltaTime; }
            inline void SetFPSCounter(int32 fpsCounter) { this->fpsCounter = fpsCounter; }
    };
}


