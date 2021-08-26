//
// Created by vitriol1744 on 26.08.2021.
//
#include "GL46Context.hpp"

#include "Graphics/API/OpenGL46/GL46RendererAPI.hpp"

namespace Vortex::Graphics
{
    GL46Context::GL46Context(GLvoid* windowHandle, IGraphicsContext* share)
    {
        Platform::EGLContextCreateInfo createInfo;
        createInfo.windowHandle = *(reinterpret_cast<Window*>(windowHandle));;
        createInfo.bitsPerPixel = 32;
        createInfo.openGLContextVersionMajor = 4;
        createInfo.openGLContextVersionMinor = 6;
        createInfo.openGLProfile = Platform::OpenGLProfile::Core;
        createInfo.sharedContext = share ? reinterpret_cast<GL46Context*>(share)->context.nativeContext : nullptr;
        
        context = Platform::EGLCreateContext(createInfo);
    
        VTCoreLogTrace("OpenGL Context Created!");
        VTCoreLogInfo("Version: {}", glGetString(GL_VERSION));
        VTCoreLogInfo("Vendor: {}", glGetString(GL_VENDOR));
        VTCoreLogInfo("Renderer: {}", glGetString(GL_RENDERER));
        VTCoreLogInfo("Shading Language Version: {}", glGetString(GL_SHADING_LANGUAGE_VERSION));

        static bool initialized = false;
        if (!initialized)
        {
            if (!LoadOpenGLFunctions()) VTCoreLogError("Failed to Load OpenGL Functions!");
            GL46RendererAPI::Initialize();
        }
        initialized = true;

        //TODO: Temporary glEnable(GL_DEPTH_TEST)!
        glEnable(GL_DEPTH_TEST);
    }

    void GL46Context::Present() noexcept
    {
        Platform::EGLSwapBuffers(context);
    }

    void GL46Context::Activate()
    {
        Platform::EGLMakeContextCurrent(context);
    }
}