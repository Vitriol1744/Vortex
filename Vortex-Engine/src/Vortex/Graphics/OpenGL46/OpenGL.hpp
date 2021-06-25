//
// Created by Vitriol1744 on 26.06.2021.
//
#pragma once

#include "Core/Macros.hpp"

#pragma region win32
#ifdef VT_PLATFORM_WINDOWS
#include "gl/GL.h"
#include "GL/wglext.h"
namespace Vortex
{
    static void* GetFunctionPointer(const char *name)
    {
        void *p = (void *)wglGetProcAddress(name);
        if(p == 0 || (p == (void*)0x1) || (p == (void*)0x2) || (p == (void*)0x3) || (p == (void*)-1) )
        {
            HMODULE module = LoadLibraryA("opengl32.dll");
            p = (void *)GetProcAddress(module, name);
        }

        return p;
    }

    static bool LoadGLFunctions()
    {


        return true;
    }
}
#endif
#pragma endregion