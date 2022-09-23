//
// Created by Vitriol1744 on 05.09.2022.
//
#ifndef VORTEX_TYPES_HPP
#define VORTEX_TYPES_HPP

#define VT_NODISCARD [[nodiscard]]

namespace Vortex
{
    using uint8     = std::uint8_t;
    using uint16    = std::uint16_t;
    using uint32    = std::uint32_t;
    using uint64    = std::uint64_t;

    using int8      = std::int8_t;
    using int16     = std::int16_t;
    using int32     = std::int32_t;
    using int64     = std::int64_t;

    using float32   = float;
    using float64   = double;

    using uint      = uint32_t;
    using strview   = std::string_view;
    using Pixel     = uint32;

    template<typename T>
    using Scope = std::unique_ptr<T>;
    template<typename T, typename... Args>
    inline constexpr Scope<T> CreateScope(Args&&... args)
    {
        return std::make_unique<T>(std::forward<Args>(args)...);
    }
    template<typename T>
    using Ref = std::shared_ptr<T>;
    template<typename T, typename... Args>
    inline constexpr Ref<T> CreateRef(Args&&... args)
    {
        return std::make_shared<T>(std::forward<Args>(args)...);
    }
}

#endif //VORTEX_TYPES_HPP
