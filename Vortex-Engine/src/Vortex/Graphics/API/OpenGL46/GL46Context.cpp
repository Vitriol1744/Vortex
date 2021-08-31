//
// Created by vitriol1744 on 26.08.2021.
//
#include "GL46Context.hpp"

#include "Graphics/API/OpenGL46/GL46RendererAPI.hpp"

namespace Vortex::Graphics
{
    GL46Context::GL46Context(NativeWindowHandleType windowHandle, IGraphicsContext* share)
    {
        Platform::EGLContextCreateInfo createInfo;
        createInfo.windowHandle = windowHandle;
        createInfo.bitsPerPixel = 32;
        createInfo.openGLContextVersionMajor = 4;
        createInfo.openGLContextVersionMinor = 6;
        createInfo.openGLProfile = Platform::OpenGLProfile::Core;
        createInfo.sharedContext = share ? reinterpret_cast<GL46Context*>(share)->context.nativeContext : nullptr;
        
        context = Platform::EGLCreateContext(createInfo);

        static bool initialized = false;
        if (!initialized) if (!GL46RendererAPI::PostInitialize()) return;
        initialized = true;

        VTCoreLogTrace("OpenGL Context Created!");
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