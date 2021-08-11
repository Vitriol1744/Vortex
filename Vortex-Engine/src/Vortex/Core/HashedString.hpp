//
// Created by Vitriol1744 on 10.08.2021.
//
#pragma once

#include "Vortex/Core/Core.hpp"

namespace Vortex
{
    using HashedID = uint64;

    struct VT_API HashedString
    {
        inline HashedString() = default;
        inline explicit HashedString(std::string_view str) : string(str), stringID(Hash(str.data())) { }

        inline void Set(std::string_view str) { string = str; stringID = Hash(str.data()); }

        inline void Concatenate(std::string_view str)
        {
            stringID = HashConcatenate(stringID, str.data());
        }
        inline static constexpr const HashedID Hash(const char* str)
        {
            return HashConcatenate(0xcbf29ce484222325, str);
        }
        inline static constexpr const HashedID HashConcatenate(HashedID base, const char* str)
        {
            return (*str) ? (HashConcatenate((base ^ *str) * 0x100000001b3, str + 1)) : base;
        }

        std::string_view string;
        HashedID stringID;
    };

    #define VT_HASH(str) HashedString::Hash(str)
}