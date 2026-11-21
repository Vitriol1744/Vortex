/*
 * Created by v1tr10l7 on 15.11.2026.
 * Copyright (c) 2024-2026, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
module;

#include <Prism/Core/Types.hpp>

#include <charconv>
#include <optional>
#include <string>
#include <string_view>
#include <system_error>

export module Vortex.Core.Formatters;

export import Prism.String.Formatter;
export import Prism.String.StringView;

// Prism::Format formats Prism's own types only: it has no floating-point
// support and knows nothing about the C++ standard library. Vortex logs both,
// so these formatters fill the gap. Importers of this module (Log.hpp) see the
// specializations; nothing else needs to be exported.
//
// Members are `inline` on purpose: in-class definitions in a named module
// aren't implicitly inline, so they'd be emitted only in Vortex-Core, which
// hides its symbols (-fvisibility=hidden), and other libraries fail to link.
namespace Prism
{
    template <>
    struct Formatter<std::string_view, char> : Formatter<StringView, char>
    {
        inline FormatAppender Format(std::string_view value, FormatContext& ctx) const
        {
            return Formatter<StringView, char>::Format(
                StringView(value.data(), value.size()), ctx);
        }
    };

    template <>
    struct Formatter<std::string, char> : Formatter<std::string_view, char>
    {
        inline FormatAppender Format(const std::string& value,
                              FormatContext&     ctx) const
        {
            return Formatter<std::string_view, char>::Format(value, ctx);
        }
    };

    /// Floating point through std::to_chars. Supports `{}` (shortest
    /// round-trip, like fmt), `{:.N}` and the f/F, e/E, g/G, a/A types.
    /// Width, fill and alignment aren't supported.
    template <typename T>
    struct FloatFormatter
    {
        inline constexpr const char* Parse(FormatParseContext& ctx)
        {
            auto it  = ctx.begin();
            auto end = ctx.end();
            if (it != end && *it == '.')
            {
                ++it;
                m_Precision = 0;
                while (it != end && *it >= '0' && *it <= '9')
                    m_Precision = m_Precision * 10 + (*it++ - '0');
            }
            if (it != end && *it != '}')
            {
                switch (*it)
                {
                    case 'f':
                    case 'F': m_Format = std::chars_format::fixed; break;
                    case 'e':
                    case 'E': m_Format = std::chars_format::scientific; break;
                    case 'g':
                    case 'G': m_Format = std::chars_format::general; break;
                    case 'a':
                    case 'A': m_Format = std::chars_format::hex; break;
                    default: return it; // Prism reports the unparsed spec
                }
                m_Upper = *it >= 'A' && *it <= 'Z';
                ++it;
            }
            return it;
        }

        inline FormatAppender Format(T value, FormatContext& ctx) const
        {
            char                 buffer[128];
            std::to_chars_result result;
            if (m_Precision >= 0)
                result = std::to_chars(buffer, buffer + sizeof(buffer), value,
                                       m_Format.value_or(
                                           std::chars_format::fixed),
                                       m_Precision);
            else if (m_Format)
                result = std::to_chars(buffer, buffer + sizeof(buffer), value,
                                       *m_Format);
            else result = std::to_chars(buffer, buffer + sizeof(buffer), value);

            usize length = result.ec == std::errc{}
                             ? static_cast<usize>(result.ptr - buffer)
                             : 0;
            if (m_Upper)
                for (usize i = 0; i < length; ++i)
                    if (buffer[i] >= 'a' && buffer[i] <= 'z') buffer[i] -= 32;

            return m_Inner.Format(StringView(buffer, length), ctx);
        }

      private:
        Formatter<StringView, char>       m_Inner{};
        std::optional<std::chars_format> m_Format{};
        int                               m_Precision = -1;
        bool                              m_Upper     = false;
    };

    template <>
    struct Formatter<float, char> : FloatFormatter<float>
    {
    };
    template <>
    struct Formatter<double, char> : FloatFormatter<double>
    {
    };
    template <>
    struct Formatter<long double, char> : FloatFormatter<long double>
    {
    };
} // namespace Prism
