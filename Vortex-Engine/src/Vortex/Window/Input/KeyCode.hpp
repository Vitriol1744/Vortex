/*
 * Created by v1tr10l7 on 18.07.2024.
 * Copyright (c) 2024-2024, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include "Vortex/Core/Types.hpp"

namespace Vortex::Input
{
    enum class KeyCode : i16
    {
        eUnknown = 0,
        eNone,
        eNum0,
        eNum1,
        eNum2,
        eNum3,
        eNum4,
        eNum5,
        eNum6,
        eNum7,
        eNum8,
        eNum9,
        eA,
        eB,
        eC,
        eD,
        eE,
        eF,
        eG,
        eH,
        eI,
        eJ,
        eK,
        eL,
        eM,
        eN,
        eO,
        eP,
        eQ,
        eR,
        eS,
        eT,
        eU,
        eV,
        eW,
        eX,
        eY,
        eZ,
        eTilde,
        eF1,
        eF2,
        eF3,
        eF4,
        eF5,
        eF6,
        eF7,
        eF8,
        eF9,
        eF10,
        eF11,
        eF12,
        eF13,
        eF14,
        eF15,
        eEscape,
        eBackspace,
        eTab,
        eCapsLock,
        eEnter,
        eReturn,
        eLShift,
        eRShift,
        eLCtrl,
        eRCtrl,
        eLAlt,
        eRAlt,
        eLSystem,
        eRSystem,
        eSpace,
        eHyphen,
        eEqual,
        eDecimal,
        eLBracket,
        eRBracket,
        eSemicolon,
        eApostrophe,
        eComma,
        ePeriod,
        eSlash,
        eBackslash,
        eUp,
        eDown,
        eLeft,
        eRight,
        eNumpad0,
        eNumpad1,
        eNumpad2,
        eNumpad3,
        eNumpad4,
        eNumpad5,
        eNumpad6,
        eNumpad7,
        eNumpad8,
        eNumpad9,
        eSeparator,
        eAdd,
        eSubtract,
        eMultiply,
        eDivide,
        eInsert,
        eDelete,
        ePageUp,
        ePageDown,
        eHome,
        eEnd,
        eScrollLock,
        eNumLock,
        ePrintScreen,
        ePause,
        eMenu,

        eKeyCount,
    };
}
