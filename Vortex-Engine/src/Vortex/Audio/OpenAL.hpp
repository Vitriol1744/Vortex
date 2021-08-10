//
// Created by Vitriol1744 on 04.08.2021.
//
#pragma once

#include "Vortex/Core/Core.hpp"

#include <string_view>

#ifdef VT_DEBUG
    #define alAssert(expr) if (expr) { } else { VT_DEBUG_BREAK; }
    #define alCall(func) Vortex::Internal::alClearError(); \
        func;                            \
        alAssert(Vortex::Internal::alLogCall(#func, __FILE__, __LINE__))

    #define alcCall(device, func) Vortex::Internal::alcClearError(device); \
        func;                            \
        alAssert(Vortex::Internal::alcLogCall(device, #func, __FILE__, __LINE__))
#else
    #define alcCall(device, func) func
    #define alCall(func) func
#endif

using ALboolean     = char;
using ALbyte        = signed char;
using ALchar        = char;
using ALenum        = int;
using ALfloat       = float;
using ALint         = int;
using ALsizei       = int;
using ALuint        = unsigned int;
using ALvoid        = void;

using ALCboolean     = char;
using ALCbyte        = signed char;
using ALCchar        = char;
using ALCenum        = int;
using ALCfloat       = float;
using ALCint         = int;
using ALCsizei       = int;
using ALCuint        = unsigned int;
using ALCvoid        = void;

class ALCdevice;

namespace Vortex::Internal
{
    extern ALvoid     alClearError();
    extern ALboolean  alLogCall(std::string_view funcname, std::string_view filename, ALuint line);
    extern ALCvoid    alcClearError(ALCdevice* device);
    extern ALCboolean alcLogCall(ALCdevice* device, std::string_view funcname, std::string_view filename, ALCuint line);
}