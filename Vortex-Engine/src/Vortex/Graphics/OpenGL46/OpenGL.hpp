//
// Created by Vitriol1744 on 27.06.2021.
//
#pragma once

#include "glad/glad.h"

#ifdef VT_PLATFORM_WINDOWS
#include <Windows.h>
#include "Gl/Gl.h"
#include "GL/wglext.h"
#elif defined(VT_PLATFORM_LINUX)
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <GL/gl.h>
#include <GL/glx.h>
#endif

namespace Vortex
{
    static bool LoadGLFunctions()
    {
        return gladLoadGL();
    }
}