//
// Created by Vitriol1744 on 22.06.2021.
//
#pragma once

#include <cstdint>
#include <memory>

namespace Vortex
{
    using uint8   = uint8_t;
    using uint16  = uint16_t;
    using uint32  = uint32_t;
    using uint64  = uint64_t;

    using int8    = int8_t;
    using int16   = int16_t;
    using int32   = int32_t;
    using int64   = int64_t;

    using float32 = float;
    using float64 = double;

    template<typename T>
    using Scope = std::unique_ptr<T>;
    template<typename T, typename ... Args>
    inline constexpr Scope<T> CreateScope(Args&& ... args)
    {
        return std::make_unique<T>(std::forward<Args>(args)...);
    }
    template<typename T>
    using Ref = std::shared_ptr<T>;
    template<typename T, typename ... Args>
    inline constexpr Ref<T> CreateRef(Args&& ... args)
    {
        return std::make_shared<T>(std::forward<Args>(args)...);
    }
}