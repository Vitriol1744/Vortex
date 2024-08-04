/*
 * Created by v1tr10l7 on 04.08.2024.
 * Copyright (c) 2024-2024, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

// Target 'Windows 10 1507 "Threshold"' or later
#undef WINVER
#undef _WIN32_WINNT
#define WINVER       0x0a000000
#define _WIN32_WINNT WINVER
#include <SdkDdkVer.h>

// We support only Unicode, and not the MBCS(Multi-Byte Character Encoding)
#ifndef UNICODE
    #define UNICODE
#endif

// We use OEM cursor resources
#define OEMRESOURCE

// Strip of some of the stuff we don't use from the Windows.h header, so we can
// have faster compile times
#define WIN32_LEAN_AND_MEAN
#define VC_EXTRALEAN
#define NOCOMM
#define NOMINMAX
#define NOMCX
#define NODEFERWINDOWPOS
#define NOGDICAPMASKS
#define NOMENUS
#define NOICONS
#define NORASTEROPS
#define NOATOM
#define NOCOLOR
#define NOCTLMGR
#define NODRAWTEXT
#define NOSERVICE
#define NOTEXTMETRIC
#define NOKANJI
#define NOHELP
#define NOPROFILER
#define NOTAPE
#define NOIMAGE
#define NOPROXYSTUB
#define NORPC
#define NOCRYPT

#ifndef STRICT
    #define STRICT
#endif
#include <Windows.h>

#ifndef VT_DIST
    #define WinAssert(x)                                                       \
        {                                                                      \
            auto result = (x);                                                 \
            if (result != 0)                                                   \
                ;                                                              \
            else                                                               \
            {                                                                  \
                VtCoreError(                                                   \
                    "Win32: '{}' wasn't sucessfull\nError Message: {}", #x,    \
                    Win32::GetErrorMessage(GetLastError()));                   \
                VtDebugBreak;                                                  \
            }                                                                  \
        }
#else
    #define WinAssert(x)                                                       \
        {                                                                      \
            auto result = (x);                                                 \
            (void)result;                                                      \
        }
#endif

#include "Vortex/Core/Utf.hpp"

namespace Vortex::Win32
{
    inline std::string GetErrorMessage(DWORD errorCode)
    {
        DWORD flags = FORMAT_MESSAGE_ALLOCATE_BUFFER
                    | FORMAT_MESSAGE_FROM_SYSTEM
                    | FORMAT_MESSAGE_IGNORE_INSERTS;
        DWORD  languageID = 0;

        TCHAR* message;
        DWORD  msgLen
            = FormatMessageW(flags, nullptr, errorCode, languageID,
                             reinterpret_cast<LPWSTR>(&message), 0, nullptr);

        if (msgLen == 0) return "No error message";
        std::string errorMessage;
        Utf32::FromWide(message, message + msgLen,
                        std::back_inserter(errorMessage));

        LocalFree(message);
        return errorMessage;
    }
}; // namespace Vortex::Win32
