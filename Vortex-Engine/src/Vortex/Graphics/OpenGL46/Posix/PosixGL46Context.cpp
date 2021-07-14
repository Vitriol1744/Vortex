//
// Created by Vitriol1744 on 29.06.2021.
//
#include "vtpch.hpp"
#include "Core/Macros.hpp"
#include <iostream>

#ifdef VT_PLATFORM_LINUX
#include "Window/Posix/X11Window.hpp"
#include "PosixGL46Context.hpp"

using glXCreateContextAttribsARBProc = GLXContext(*)(Display*, GLXFBConfig, GLXContext, Bool, const int*);

namespace Vortex::Graphics
{
    GL46Context::GL46Context(void* windowHandle, IGraphicsContext* share) : IGraphicsContext(windowHandle)
    {
        XInitThreads();
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
        //TODO: Resource sharing!
        static int contextAttributes[] =
        {
            GLX_CONTEXT_MAJOR_VERSION_ARB, 4,
            GLX_CONTEXT_MINOR_VERSION_ARB, 6,
            GLX_CONTEXT_PROFILE_MASK_ARB,  GLX_CONTEXT_CORE_PROFILE_BIT_ARB,
            None
        };

        GLXContext sharedContext = nullptr;
        sharedContext = share ? dynamic_cast<GL46Context*>(share)->context : nullptr;
        VT_CORE_LOG_WARN("WARN!");

        if (framebufferConfig != nullptr)
            context = glXCreateContextAttribsARB(display, framebufferConfig[0], sharedContext, true, contextAttributes);
    
        VT_CORE_ASSERT(context != nullptr);

        glXMakeCurrent(display, window, context);
    
        int major = 0, minor = 0;
        glGetIntegerv(GL_MAJOR_VERSION, &major);
        glGetIntegerv(GL_MINOR_VERSION, &minor);
    
        VT_CORE_LOG_INFO("OpenGL Context Created!");
        VT_CORE_LOG_INFO("Version: {}.{}", major, minor);
        VT_CORE_LOG_INFO("Vendor: {}", glGetString(GL_VENDOR));
        VT_CORE_LOG_INFO("Renderer: {}", glGetString(GL_RENDERER));

        if (!LoadGLFunctions()) VT_CORE_LOG_ERROR("Failed to Load GL Functions!");
    }
    
    GL46Context::~GL46Context() noexcept
    {
        glXDestroyContext(display, context);
    }

    void GL46Context::Present() noexcept
    {
        glXSwapBuffers(display, window);
    }

    void GL46Context::Activate()
    {
        glXMakeCurrent(display, window, context);
    }
}
#endif
