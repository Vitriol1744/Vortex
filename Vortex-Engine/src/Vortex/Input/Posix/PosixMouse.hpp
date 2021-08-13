//
// Created by Vitriol1744 on 13.08.2021.
//
#pragma once

#include "Vortex/Core/Core.hpp"

#include "Vortex/Input/Mouse.hpp"

namespace Vortex::Input
{
    class VT_API MouseImpl : public Mouse
    {
        public:
            virtual bool IsButtonPressedImpl(MouseCode mousecode) override;
            virtual Math::Vec2i CursorPositionImpl() override;

            virtual void SetCursorPositionImpl(int32 x, int32 y) override;
    };
}
