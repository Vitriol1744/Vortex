/*
 * Created by v1tr10l7 on 17.07.2024.
 * Copyright (c) 2024-2024, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

namespace Vortex
{
    using Path     = std::filesystem::path;
    using PathView = const std::filesystem::path&;

    using usize    = std::size_t;
    using isize    = std::make_signed_t<size_t>;

    using u8       = uint8_t;
    using u16      = uint16_t;
    using u32      = uint32_t;
    using u64      = uint64_t;

    using i8       = int8_t;
    using i16      = int16_t;
    using i32      = int32_t;
    using i64      = int64_t;

    using f32      = float;
    using f64      = double;

    inline u64 operator""_kb(unsigned long long kilobytes)
    {
        return 1024 * kilobytes;
    }
    inline u64 operator""_mb(unsigned long long megabytes)
    {
        return megabytes * 1024_kb;
    }
    inline u64 operator""_gb(unsigned long long gigabytes)
    {
        return 1024_mb * gigabytes;
    }

    template <typename T>
    using Scope = std::unique_ptr<T>;
    template <typename T, typename... Args>
    inline constexpr Scope<T> CreateScope(Args&&... args)
    {
        return std::make_unique<T>(std::forward<Args>(args)...);
    }
    template <typename T>
    using Ref = std::shared_ptr<T>;
    template <typename T, typename... Args>
    inline constexpr Ref<T> CreateRef(Args&&... args)
    {
        return std::make_shared<T>(std::forward<Args>(args)...);
    }
    template <typename T>
    using WeakRef = std::weak_ptr<T>;

    struct Version
    {
        u16 Major           = 0;
        u16 Minor           = 0;
        u16 Patch           = 0;

        constexpr Version() = default;
        constexpr Version(u16 major, u16 minor, u16 patch)
            : Major(major)
            , Minor(minor)
            , Patch(patch)
        {
        }

        constexpr operator u64() const
        {
            return (static_cast<u64>(Major) << 48)
                 | (static_cast<u64>(Minor) << 32)
                 | (static_cast<u64>(Patch) << 16ull);
        }

        friend std::ostream& operator<<(std::ostream&  os,
                                        const Version& version)
        {
            os << version.Major << "." << version.Minor << "." << version.Patch;
            return os;
        }
    };

}; // namespace Vortex

template <>
struct fmt::formatter<Vortex::Version> : fmt::formatter<std::string>
{
    template <typename FormatContext>
    auto format(const Vortex::Version& version, FormatContext& ctx) const
    {

        return fmt::formatter<std::string>::format(
            fmt::format("{}.{}.{}", version.Major, version.Minor,
                        version.Patch),
            ctx);
    }
};

template <>
struct fmt::formatter<Vortex::Path> : fmt::formatter<std::string>
{
    template <typename FormatContext>
    auto format(const Vortex::Path& path, FormatContext& ctx) const
    {
        return fmt::formatter<std::string>::format(
            fmt::format("{}", path.c_str()), ctx);
    }
};
