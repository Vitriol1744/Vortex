//
// Created by Vitriol1744 on 13.08.2021.
//
#pragma once

#include "Vortex/Core/Core.hpp"

#include "Vortex/Input/Keyboard.hpp"

namespace Vortex::Input
{
    class KeyboardImpl : public Keyboard
    {
        public:
            virtual bool IsKeyPressedImpl(KeyCode keycode) const override;
    };
}

