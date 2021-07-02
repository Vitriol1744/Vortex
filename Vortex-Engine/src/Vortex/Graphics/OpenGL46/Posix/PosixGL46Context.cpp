//
// Created by Vitriol1744 on 29.06.2021.
//
#include "vtpch.hpp"
#include "Core/Macros.hpp"

#ifdef VT_PLATFORM_LINUX
#include "Window/Posix/PosixWindow.hpp"
#include "PosixGL46Context.hpp"

using glXCreateContextAttribsARBProc = GLXContext(*)(Display*, GLXFBConfig, GLXContext, Bool, const int*);

namespace Vortex
{
    GL46Context::~GL46Context()
    {

    }

    void GL46Context::Initialize(void* windowHandle)
    {
        display = WindowImpl::display;
        window = *(reinterpret_cast<Window*>(windowHandle));

        static int visualAttributes[] =
        {
            GLX_RENDER_TYPE, GLX_RGBA_BIT,
            GLX_DRAWABLE_TYPE, GLX_WINDOW_BIT,
            GLX_DOUBLEBUFFER, true,
            GLX_RED_SIZE, 1,
            GLX_GREEN_SIZE, 1,
            GLX_BLUE_SIZE, 1,
            None
        };

        GLXFBConfig* framebufferConfig = glXChooseFBConfig(display, DefaultScreen(display), visualAttributes, nullptr);
        VT_CORE_ASSERT(framebufferConfig != nullptr);

        glXCreateContextAttribsARBProc glXCreateContextAttribsARB = nullptr;
        glXCreateContextAttribsARB = (glXCreateContextAttribsARBProc)glXGetProcAddress((const GLubyte*)"glXCreateContextAttribsARB");
        VT_CORE_ASSERT(glXCreateContextAttribsARB != nullptr)

        static int contextAttributes[] =
        {
            GLX_CONTEXT_MAJOR_VERSION_ARB, 4,
            GLX_CONTEXT_MINOR_VERSION_ARB, 2,
            None
        };
        context = glXCreateContextAttribsARB(display, framebufferConfig[0], nullptr, true, contextAttributes);
        VT_CORE_ASSERT(context != nullptr);

        glXMakeCurrent(display, window, context);
    }

    void GL46Context::Present()
    {

    }
}
#endif
