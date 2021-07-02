//
// Created by Vitriol1744 on 27.06.2021.
//
#pragma once

#include "Core/Core.hpp"

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

#include <KHR/khrplatform.h>

#pragma region glTypes
using GLsizeiptr = khronos_ssize_t;
#pragma endregion

#pragma region glMacros
#define GL_ARRAY_BUFFER 0x8892
#define GL_STATIC_DRAW 0x88E4
#pragma endregion

#pragma region glFunctions
using PFNGLBINDBUFFERPROC = void(*)(GLenum target, GLuint buffer);
static PFNGLBINDBUFFERPROC glBindBuffer;

using PFNGLBUFFERDATAPROC = void(*)(GLenum target, GLsizeiptr size, const GLvoid * data, GLenum usage);
static PFNGLBUFFERDATAPROC glBufferData;

using PFNGLCREATEBUFFERSPROC = void(*)(GLsizei n, GLuint* buffers);
static PFNGLCREATEBUFFERSPROC glCreateBuffers;

using PFNGLDELETEBUFFERSPROC = void(*)(GLsizei n, const GLuint* buffers);
static PFNGLDELETEBUFFERSPROC glDeleteBuffers;
#pragma endregion

namespace Vortex
{
    #ifdef VT_PLATFORM_WINDOWS
    static void* GetProcAddress(const char* name)
    {
        void* p = (void*)wglGetProcAddress(name);
        if(p == 0 || (p == (void*)0x1) || (p == (void*)0x2) || (p == (void*)0x3) || (p == (void*)-1))
        {
            HMODULE module = LoadLibraryA("opengl32.dll");
            p = (void *)GetProcAddress(module, name);
        }

        return p;
    }
    #elif defined(VT_PLATFORM_LINUX)
    static void* GetProcAddress(const unsigned char* name)
    {
        void* p = (void*)glXGetProcAddress(name);
        if(p == 0 || (p == (void*)0x1) || (p == (void*)0x2) || (p == (void*)0x3) || (p == (void*)-1))
        {
            //HMODULE module = LoadLibraryA("opengl32.dll");
            //p = (void *)GetProcAddress(module, name);
            VT_CORE_LOG_ERROR("Couldn't load GL function!");
        }

        return p;
    }
    #endif

    static bool LoadGLFunctions()
    {

        return true;
    }
}