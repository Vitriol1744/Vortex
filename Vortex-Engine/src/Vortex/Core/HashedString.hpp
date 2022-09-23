//
// Created by Vitriol1744 on 05.09.2022.
//
#ifndef VORTEX_HASHED_STRING_HPP
#define VORTEX_HASHED_STRING_HPP

#include "Vortex/Core/Export.hpp"
#include "Vortex/Core/Types.hpp"

namespace Vortex
{
    using HashedID = uint64;

    struct HashedString
    {
        inline HashedString() = default;
        inline explicit HashedString(std::string_view str)
            : string(str), string_id(Hash(str.data())) { }

        inline void Set(std::string_view str) { string = str; string_id = Hash(str.data()); }
        inline void Concatenate(std::string_view str)
        {
            string_id = HashConcatenate(string_id, str.data());
        }
        inline static constexpr const HashedID Hash(const char* str)
        {
            return HashConcatenate(0xcbf29ce484222325, str);
        }
        inline static constexpr const HashedID HashConcatenate(HashedID base, const char* str)
        {
            return (*str != '\0') ? (HashConcatenate((base ^ *str) * 0x100000001b3, str + 1)) : base;
        }

        std::string_view string;
        HashedID string_id;
    };

}

#define VT_HASH(str) Vortex::HashedString::Hash(str)

#endif //VORTEX_HASHED_STRING_HPP
