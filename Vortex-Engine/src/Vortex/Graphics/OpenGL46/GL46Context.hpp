//
// Created by Vitriol1744 on 24.06.2021.
//
#pragma once

#ifdef VT_PLATFORM_WINDOWS
    #include "Win32/Win32GL46Context.hpp"
#elif defined(VT_PLATFORM_LINUX)
    #include "Posix/PosixGL46Context.hpp"
#endif
