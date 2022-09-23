//
// Created by Vitriol1744 on 05.09.2022.
//
#ifndef VORTEX_CORE_HPP
#define VORTEX_CORE_HPP

#include "Vortex/Core/Assertions.hpp"
#include "Vortex/Core/Export.hpp"
#include "Vortex/Core/HashedString.hpp"
#include "Vortex/Core/LoggingManager.hpp"
#include "Vortex/Core/Math/Math.hpp"
#include "Vortex/Core/NonCopyable.hpp"
#include "Vortex/Core/Singleton.hpp"
#include "Vortex/Core/Types.hpp"

namespace Vortex
{
    constexpr const uint8 vortexVersion_Major = 0;
    constexpr const uint8 vortexVersion_Minor = 1;
    constexpr const uint8 vortexVersion_Patch = 0;
}

#define VT_BIT(n) (1 << n)

#ifndef __FUNCSIG__
    #define __FUNCSIG__ __PRETTY_FUNCTION__
#endif

#endif //VORTEX_CORE_HPP
