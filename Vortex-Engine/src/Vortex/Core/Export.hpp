//
// Created by Vitriol1744 on 24.06.2021.
//
#pragma once

#ifdef VT_BUILD_DLL
    #define VT_API _declspec(dllexport)
#else
    #define VT_API _declspec(dllimport)
#endif