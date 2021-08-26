//
// Created by Vitriol1744 on 21.08.2021.
//
#pragma once

#include "Vortex/Core/Core.hpp"

constexpr const int DEFAULT_OPENGL_CONTEXT_VERSION_MAJOR = 4;
constexpr const int DEFAULT_OPENGL_CONTEXT_VERSION_MINOR = 6;

namespace Vortex::Platform
{
    enum class OpenGLProfile
    {
        Compatibility,
        Core,
        
        Default = Core
    };
    
    struct VT_API EGLContextCreateInfo
    {
        void* windowHandle;
        int32 bitsPerPixel;

        uint32 openGLContextVersionMajor = 4;
        uint32 openGLContextVersionMinor = 6;
        OpenGLProfile openGLProfile      = OpenGLProfile::Default;
    };
    
    struct VT_API EGLContext
    {
        Ref<void> nativeContext;
        EGLContext() = default;
        inline EGLContext(Ref<void> context) { nativeContext = context;  }
        virtual ~EGLContext() = default;
    };

    EGLContext EGLCreateContext(EGLContextCreateInfo& createInfo);
    void EGLSwapBuffers(EGLContext& context);
    void EGLMakeContextCurrent(EGLContext& context);
}