//
// Created by Vitriol1744 on 20.08.2021.
//
#pragma once

#include "Vortex/Core/Core.hpp"

namespace Vortex::Graphics
{
    class VT_API Layer
    {
        public:
            virtual ~Layer() = default;

            virtual void OnAttach() { }
            virtual void OnDetach() { }
            virtual void OnUpdate() { }
            virtual void OnRender() { }
    };
}

