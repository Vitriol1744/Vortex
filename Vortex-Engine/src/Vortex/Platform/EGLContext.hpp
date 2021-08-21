//
// Created by Vitriol1744 on 21.08.2021.
//
#pragma once

#include "Vortex/Core/Core.hpp"

#define DEFAULT_OPENGL_CONTEXT_VERSION_MAJOR 4
#define DEFAULT_OPENGL_CONTEXT_VERSION_MINOR 6
#define DEFAULT_OPENGL_PROFILE OpenGLProfile::Core

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
        uint32 openGLContextVersionMajor = DEFAULT_OPENGL_CONTEXT_VERSION_MAJOR;
        uint32 openGLContextVersionMinor = DEFAULT_OPENGL_CONTEXT_VERSION_MINOR;
        OpenGLProfile openGLProfile      = DEFAULT_OPENGL_PROFILE;
    };
    
    struct VT_API EGLContext
    {
        EGLContextCreateInfo createInfo;
        void* nativeContext;
    };
}