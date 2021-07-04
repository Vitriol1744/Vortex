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
    GL46Context::GL46Context(void* windowHandle) : IGraphicsContext(windowHandle)
    {
        display = WindowImpl::GetDisplay();
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
    
        int framebufferConfigsCount = 0;
        GLXFBConfig* framebufferConfig = glXChooseFBConfig(display, DefaultScreen(display), visualAttributes, &framebufferConfigsCount);
        VT_CORE_ASSERT(framebufferConfig != nullptr);
    
        glXCreateContextAttribsARBProc glXCreateContextAttribsARB = nullptr;
        glXCreateContextAttribsARB = (glXCreateContextAttribsARBProc)glXGetProcAddress((const GLubyte*)"glXCreateContextAttribsARB");
        VT_CORE_ASSERT(glXCreateContextAttribsARB != nullptr)
    
        static int contextAttributes[] =
        {
            GLX_CONTEXT_MAJOR_VERSION_ARB, 4,
            GLX_CONTEXT_MINOR_VERSION_ARB, 6,
            None
        };
        if (framebufferConfig != nullptr)
            context = glXCreateContextAttribsARB(display, framebufferConfig[0], nullptr, true, contextAttributes);
    
        VT_CORE_ASSERT(context != nullptr);
    
        glXMakeCurrent(display, window, context);
    
        int major = 0, minor = 0;
        glGetIntegerv(GL_MAJOR_VERSION, &major);
        glGetIntegerv(GL_MINOR_VERSION, &minor);
    
        VT_CORE_LOG_INFO("OpenGL Context Created!");
        VT_CORE_LOG_INFO("Version: {}.{}", major, minor);
        VT_CORE_LOG_INFO("Vendor: {}", glGetString(GL_VENDOR));
        VT_CORE_LOG_INFO("Renderer: {}", glGetString(GL_RENDERER));
    }
    
    GL46Context::~GL46Context()
    {
        glXDestroyContext(display, context);
    }

    void GL46Context::Present()
    {
        glXSwapBuffers(display, window);
    }
}
#endif
