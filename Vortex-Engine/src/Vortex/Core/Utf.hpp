/*
 * Created by v1tr10l7 on 01.08.2024.
 * Copyright (c) 2024-2024, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include "Vortex/Core/Types.hpp"

namespace Vortex
{
    template <u32 N>
    class Utf;

    template <>
    class Utf<8>
    {
    };

    template <>
    class Utf<32>
    {
      public:
        template <typename In, typename Out>
        static Out ToWide(In begin, In end, Out output, wchar_t replacement);

        template <typename In>
        static u32 DecodeAnsi(In input, const std::locale& locale);
        template <typename In>
        static u32 DecodeWide(In input);

        template <typename Out>
        static Out EncodeAnsi(u32 codepoint, Out output, char replacement,
                              const std::locale& locale);
        template <typename Out>
        static Out EncodeWide(u32 codepoint, Out output,
                              wchar_t replacement = 0);
    };

    using Utf8  = Utf<8>;
    using Utf16 = Utf<16>;
    using Utf32 = Utf<32>;
}; // namespace Vortex

#include "Vortex/Core/Utf.inl"
