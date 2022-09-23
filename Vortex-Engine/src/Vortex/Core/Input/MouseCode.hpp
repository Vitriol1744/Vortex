//
// Created by Vitriol1744 on 07.09.2022.
//
#ifndef VORTEX_MOUSECODE_HPP
#define VORTEX_MOUSECODE_HPP

#include "Vortex/Core/Core.hpp"

namespace Vortex::Input
{
    enum class MouseCode : int8
    {
        Unknown = 0,
        None,
        Left,
        Right,
        Middle,
        X1,
        X2,

        ButtonsCount
    };
}

#endif //VORTEX_MOUSECODE_HPP
