//
// Created by Vitriol1744 on 26.08.2021.
//
#pragma once

#include "Vortex/Core/Core.hpp"

namespace Vortex
{
    template <typename Out>
    Out EncodeWide(uint32 codepoint, Out output, wchar_t replacement)
    {
        switch (sizeof(wchar_t))
        {
            case 4: *output++ = static_cast<wchar_t>(codepoint); break;

            default:
            {
                if ((codepoint <= 0xFFFF) && ((codepoint < 0xD800) || (codepoint > 0xDFFF))) 
                    *output++ = static_cast<wchar_t>(codepoint);
                else if (replacement) *output++ = replacement;
                
                break;
            }
        }

        return output;
    }

    template <typename In, typename Out>
    Out ToWideString(In begin, In end, Out output, wchar_t replacement)
    {
        while (begin < end) output = EncodeWide(*begin++, output, replacement);

        return output;
    }
}