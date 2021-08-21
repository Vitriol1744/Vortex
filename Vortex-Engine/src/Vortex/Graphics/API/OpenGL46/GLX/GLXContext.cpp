//
// Created by Vitriol1744 on 29.06.2021.
//
#include "Vortex/Core/Platform.hpp"

#ifdef VT_PLATFORM_LINUX
#include "Vortex/Graphics/Window/Posix/X11Window.hpp"
#include "GLXContext.hpp"
#include "Graphics/API/OpenGL46/GL46RendererAPI.hpp"

using glXCreateContextAttribsARBProc = GLXContext(*)(Display*, GLXFBConfig, GLXContext, GLint, const GLint*);

PFNGLXSWAPINTERVALMESAPROC SwapInterval = nullptr;

#include "Vortex/Platform/X11/X11.hpp"

namespace Vortex::Graphics
{
    GLboolean GL46Context::initialized = false;

    GL46Context::GL46Context(GLvoid* windowHandle, IGraphicsContext* share)
    {
        XInitThreads();
        display = WindowImpl::GetDisplay();
        window = *(reinterpret_cast<Window*>(windowHandle));

        //TODO: SetBitsPerPixel to Custom Value!
        static GLint visualAttributes[] =
        {
            GLX_X_RENDERABLE        , True,
            GLX_DRAWABLE_TYPE       , GLX_WINDOW_BIT,
            GLX_RENDER_TYPE         , GLX_RGBA_BIT,
            GLX_X_VISUAL_TYPE       , GLX_TRUE_COLOR,
            GLX_RED_SIZE            , 8,
            GLX_GREEN_SIZE          , 8,
            GLX_BLUE_SIZE           , 8,
            GLX_ALPHA_SIZE          , 8,
            GLX_DEPTH_SIZE          , 24,
            GLX_STENCIL_SIZE        , 8,
            GLX_DOUBLEBUFFER        , True,
            GLX_SAMPLE_BUFFERS      , 1,
            GLX_SAMPLES             , 4,
            XNone
        };
    
        int framebufferConfigsCount = 0;
        GLXFBConfig* framebufferConfig = glXChooseFBConfig(display, DefaultScreen(display), visualAttributes, &framebufferConfigsCount);
        VT_CORE_ASSERT(framebufferConfig != nullptr);
    
        glXCreateContextAttribsARBProc glXCreateContextAttribsARB = nullptr;
        glXCreateContextAttribsARB = (glXCreateContextAttribsARBProc)glXGetProcAddress((const GLubyte*)"glXCreateContextAttribsARB");
        VT_CORE_ASSERT(glXCreateContextAttribsARB != nullptr)
        
        //TODO: Resource sharing!
        static GLint contextAttributes[] =
        {
            GLX_CONTEXT_MAJOR_VERSION_ARB, 4,
            GLX_CONTEXT_MINOR_VERSION_ARB, 6,
            GLX_CONTEXT_PROFILE_MASK_ARB,  GLX_CONTEXT_CORE_PROFILE_BIT_ARB,
            XNone
        };

        GLXContext sharedContext = share ? dynamic_cast<GL46Context*>(share)->context : nullptr;

        if (framebufferConfig) context = glXCreateContextAttribsARB(display, framebufferConfig[0], sharedContext, true, contextAttributes);
    
        VT_CORE_ASSERT(context != nullptr);

        glXMakeCurrent(display, window, context);
    
        VTCoreLogTrace("OpenGL Context Created!");
        VTCoreLogInfo("Version: {}", glGetString(GL_VERSION));
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
        //TODO: Temporary glEnable(GL_DEPTH_TEST)!
        glEnable(GL_DEPTH_TEST);
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
