//
// Created by vitriol1744 on 08.07.2021.
//
#pragma once

#include "Core/Core.hpp"

namespace Vortex::Input
{
    enum class MouseCode : int8
    {
        Unknown = -1,
        None,
        Left,
        Right,
        Middle,
        X1,
        X2,

        ButtonsCount
    };
}