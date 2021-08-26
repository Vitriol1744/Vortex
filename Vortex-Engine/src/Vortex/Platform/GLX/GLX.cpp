//
// Created by vitriol1744 on 26.08.2021.
//
#include "Vortex/Platform/EGL.hpp"
#include "Vortex/Platform/X11/X11.hpp"

#include "Vortex/Graphics/API/OpenGL46/OpenGL.hpp"

#include "Vortex/Graphics/Window/Posix/X11Window.hpp"

namespace Vortex::Platform
{
    using glXCreateContextAttribsARBProc = GLXContext(*)(Display*, GLXFBConfig, GLXContext, GLint, const GLint*);
    PFNGLXSWAPINTERVALMESAPROC SwapInterval = nullptr;

    struct GLXContext
    {
        Display* display = nullptr;
        Window window = 0;
        __GLXcontextRec* context;
        
        ~GLXContext()
        {
            glXDestroyContext(display, context);
        }
    };
    
    EGLContext EGLCreateContext(EGLContextCreateInfo& createInfo)
    {
        if (!SwapInterval)
        {
            SwapInterval = (PFNGLXSWAPINTERVALMESAPROC)glXGetProcAddress(reinterpret_cast<const unsigned char*>("glXSwapIntervalMESA"));
            SwapInterval(0);
        }
        
        Ref<GLXContext> context = CreateRef<GLXContext>();
        context->window = createInfo.windowHandle;
        context->display = WindowImpl::GetDisplay();
        
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
        GLXFBConfig* framebufferConfig = glXChooseFBConfig(context->display, DefaultScreen(context->display), visualAttributes, &framebufferConfigsCount);
        VT_CORE_ASSERT(framebufferConfig != nullptr);
    
        Ref<GLXContext> sharedContext = std::reinterpret_pointer_cast<GLXContext>(createInfo.sharedContext);
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

        if (framebufferConfig) context->context = glXCreateContextAttribsARB(context->display, framebufferConfig[0], sharedContext ? sharedContext->context : nullptr, true, contextAttributes);
    
        VT_CORE_ASSERT(context != nullptr);

        glXMakeCurrent(context->display, context->window, context->context);
        
        return { std::reinterpret_pointer_cast<EGLContext>(context) };
    }
    void EGLSwapBuffers(EGLContext& _context)
    {
        Ref<GLXContext> context = std::reinterpret_pointer_cast<GLXContext>(_context.nativeContext);
        glXSwapBuffers(context->display, context->window);
    }
    void EGLMakeContextCurrent(EGLContext& _context)
    {
        Ref<GLXContext> context = std::reinterpret_pointer_cast<GLXContext>(_context.nativeContext);
        glXMakeCurrent(context->display, context->window, context->context);
    }
}
