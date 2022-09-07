//
// Created by Vitriol1744 on 05.09.2022.
//
#ifndef VORTEX_EXPORT_HPP
#define VORTEX_EXPORT_HPP

#define VT_STATIC_LIBRARY
#ifndef VT_STATIC_LIBRARY
    #ifdef _MSC_VER
        #ifdef VT_BUILD_DLL
            #define VT_API _declspec(dllexport)
        #else
            #define VT_API _declspec(dllimport)
        #endif
    #else
        #ifdef VT_BUILD_DLL
            #define VT_API __attribute__((visibility("default")))
        #else
            #define VT_API __attribute__((visibility("hidden")))
        #endif
    #endif
#else
    #define VT_API
#endif

#endif //VORTEX_EXPORT_HPP
