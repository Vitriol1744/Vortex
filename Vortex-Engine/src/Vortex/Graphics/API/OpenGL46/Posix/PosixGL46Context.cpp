//
// Created by Vitriol1744 on 29.06.2021.
//
#include "Vortex/Core/Platform.hpp"

#ifdef VT_PLATFORM_LINUX
#include "Window/Posix/X11Window.hpp"
#include "PosixGL46Context.hpp"
#include "Graphics/API/OpenGL46/GL46RendererAPI.hpp"

using glXCreateContextAttribsARBProc = GLXContext(*)(Display*, GLXFBConfig, GLXContext, GLint, const GLint*);

PFNGLXSWAPINTERVALMESAPROC SwapInterval = nullptr;

namespace Vortex::Graphics
{
    GLboolean GL46Context::initialized = false;

    GL46Context::GL46Context(GLvoid* windowHandle, IGraphicsContext* share)
    {
        XInitThreads();
        display = WindowImpl::Display();
        window = *(reinterpret_cast<Window*>(windowHandle));

        //TODO: SetBitsPerPixel to Custom Value!
        static GLint visualAttributes[] =
        {
            GLX_RENDER_TYPE, GLX_RGBA_BIT,
            GLX_DRAWABLE_TYPE, GLX_WINDOW_BIT,
            GLX_DOUBLEBUFFER, true,
            GLX_RED_SIZE, 1,
            GLX_GREEN_SIZE, 1,
            GLX_BLUE_SIZE, 1,
            0L
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
            0L
        };

        GLXContext sharedContext = nullptr;
        sharedContext = share ? dynamic_cast<GL46Context*>(share)->context : nullptr;

        if (framebufferConfig != nullptr) context = glXCreateContextAttribsARB(display, framebufferConfig[0], sharedContext, true, contextAttributes);
    
        VT_CORE_ASSERT(context != nullptr);

        glXMakeCurrent(display, window, context);
    
        GLint major = 0, minor = 0;
        glGetIntegerv(GL_MAJOR_VERSION, &major);
        glGetIntegerv(GL_MINOR_VERSION, &minor);
    
        VTCoreLogTrace("OpenGL Context Created!");
        VTCoreLogInfo("Version: {}.{}", major, minor);
        VTCoreLogInfo("Vendor: {}", glGetString(GL_VENDOR));
        VTCoreLogInfo("Renderer: {}", glGetString(GL_RENDERER));
        VTCoreLogInfo("Shading Language Version: {}", glGetString(GL_SHADING_LANGUAGE_VERSION));

        if (!initialized)
        {
            if (!LoadOpenGLFunctions()) VTCoreLogError("Failed to Load OpenGL Functions!");
            SwapInterval = (PFNGLXSWAPINTERVALMESAPROC)glXGetProcAddress(reinterpret_cast<const unsigned char*>("glXSwapIntervalMESA"));
            GL46RendererAPI::Initialize();
        }
        initialized = true;

        SwapInterval(0); //TODO: Toggleable vsync
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
