/*
 * Created by v1tr10l7 on 01.08.2024.
 * Copyright (c) 2024-2024, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#include "Vortex/Core/Utf.hpp"

namespace Vortex
{
    template <typename In, typename Out>
    Out Utf<32>::ToWide(In begin, In end, Out output, wchar_t replacement)
    {
        while (begin < end) output = EncodeWide(*begin++, output, replacement);

        return output;
    }

    template <typename In>
    u32 Utf<32>::DecodeAnsi(In input, const std::locale& locale)
    {
        const auto& facet = std::use_facet<std::ctype<wchar_t>>(locale);

        return static_cast<u32>(facet.widen(input));
    }

    template <typename In>
    u32 Utf<32>::DecodeWide(In input)
    {
        // The encoding of wide characters is not well defined and is left to
        // the system; however we can safely assume that it is UCS-2 on Windows
        // and UCS-4 on Unix systems. In both cases, a simple copy is enough
        // (UCS-2 is a subset of UCS-4, and UCS-4 *is* UTF-32).

        return static_cast<u32>(input);
    }

    template <typename Out>
    Out Utf<32>::EncodeAnsi(u32 codepoint, Out output, char replacement,
                            const std::locale& locale)
    {
        const auto& facet = std::use_facet<std::ctype<wchar_t>>(locale);
        *output++ = facet.narrow(static_cast<wchar_t>(codepoint), replacement);

        return output;
    }

    template <typename Out>
    Out Utf<32>::EncodeWide(u32 codepoint, Out output, wchar_t replacement)
    {
        if constexpr (sizeof(wchar_t) == 4)
            *output++ = static_cast<wchar_t>(codepoint);
        else if ((codepoint <= 0xffff)
                 && ((codepoint < 0xd800) || (codepoint > 0xdfff)))
            *output++ = static_cast<wchar_t>(codepoint);
        else if (replacement) *output++ = replacement;

        return output;
    }
}; // namespace Vortex
