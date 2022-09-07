//
// Created by Vitriol1744 on 07.09.2022.
//

#include "Vortex/Core/Export.hpp"
#include "Vortex/Core/Types.hpp"

namespace Vortex::Utf
{
    template<typename Out>
    Out EncodeWide(uint32 code_point, Out output, wchar_t replacement)
    {
        if (sizeof(wchar_t) == 4) *output = static_cast<wchar_t>(code_point);
        else
        {
            if ((code_point <= 0xffff) && ((code_point < 0xd800) || (code_point > 0xdfff)))
                *output++ = static_cast<wchar_t>(code_point);
            else if (replacement != '\0') *output++ = replacement;
        }

        return output;
    }

    template<typename In, typename Out>
    Out ToWideString(In begin, In end, Out output, wchar_t replacement)
    {
        while (begin < end) output = EncodeWide(*begin++, output, replacement);

        return output;
    }
}