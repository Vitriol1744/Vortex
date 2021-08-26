//
// Created by Vitriol1744 on 29.06.2021.
//
#include "vtpch.hpp"
#include "Vortex/Platform/Platform.hpp"

#include <iostream>

#ifdef VT_PLATFORM_WINDOWS
#include "Win32GL46Context.hpp"

#include "Vortex/Platform/EGL.hpp"

namespace Vortex::Graphics
{
    GL46Context::GL46Context(GLvoid* windowHandle, uint32 bitsPerPixel, IGraphicsContext* share)
    {
        using namespace Platform;
        EGLContextCreateInfo eglCreateInfo;
        eglCreateInfo.bitsPerPixel = 32;
        eglCreateInfo.openGLContextVersionMajor = 4;
        eglCreateInfo.openGLContextVersionMinor = 6;
        eglCreateInfo.openGLProfile = OpenGLProfile::Core;
        eglCreateInfo.windowHandle = windowHandle;

        context = EGLCreateContext(eglCreateInfo);

        VTCoreLogTrace("OpenGL Context Created!");
        VTCoreLogInfo("Version: {}", glGetString(GL_VERSION));
        VTCoreLogInfo("Vendor: {}", glGetString(GL_VENDOR));
        VTCoreLogInfo("Renderer: {}", glGetString(GL_RENDERER));
        VTCoreLogInfo("Shading Language Version: {}", glGetString(GL_SHADING_LANGUAGE_VERSION));

        VT_CORE_ASSERT(LoadOpenGLFunctions() == true);

        glEnable(GL_DEPTH_TEST);
    }

    void GL46Context::Present()
    {
        Platform::EGLSwapBuffers(context);
    }

    void GL46Context::Activate()
    {
        Platform::EGLMakeContextCurrent(context);
    }
}
#endif
