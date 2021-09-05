//
// Created by vitriol1744 on 08.07.2021.
//
#pragma once

#include "Vortex/Core/Core.hpp"

namespace Vortex::Input
{
    enum class MouseCode : int8
    {
        eUnknown = 0,
        eNone,
        eLeft,
        eRight,
        eMiddle,
        eX1,
        eX2,

        eButtonsCount
    };
}