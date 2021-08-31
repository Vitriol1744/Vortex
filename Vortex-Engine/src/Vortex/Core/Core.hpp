//
// Created by Vitriol1744 on 22.06.2021.
//
#pragma once

#include "Vortex/Core/Export.hpp"
#include "Vortex/Platform/Platform.hpp"
#include "Vortex/Core/Assertions.hpp"
#include "Vortex/Core/LoggingManager.hpp"
#include "Vortex/Core/Types.hpp"
#include "Vortex/Core/Singleton.hpp"
#include "Vortex/Core/NonCopyable.hpp"
#include "Vortex/Core/Math/Math.hpp"
#include "Vortex/Core/HashedString.hpp"

#define VT_NODISCARD [[nodiscard]]

namespace Vortex
{
    constexpr const uint8 vortexVersion_Major = 0;
    constexpr const uint8 vortexVersion_Minor = 1;
    constexpr const uint8 vortexVersion_Patch = 0;
}

#ifndef __FUNCSIG__
    #define __FUNCSIG__ __PRETTY_FUNCTION__
#endif