//
// Created by vitriol1744 on 11.07.2021.
//
#include "OpenGL.hpp"

namespace Vortex::Graphics
{
    #pragma region initializing opengl functions
    #define VT_GL_INIT_FUNC(name) PFNGL##name##PROC gl##name = nullptr
    VT_GL_INIT_FUNC(AttachShader);
    VT_GL_INIT_FUNC(BindBuffer);
    VT_GL_INIT_FUNC(BindSampler);
    VT_GL_INIT_FUNC(BindTexture);
    VT_GL_INIT_FUNC(BindVertexArray);
    VT_GL_INIT_FUNC(BlendEquationSeparate);
    VT_GL_INIT_FUNC(BlendFuncSeparate);
    VT_GL_INIT_FUNC(BufferData);
    VT_GL_INIT_FUNC(BufferSubData);
    VT_GL_INIT_FUNC(CopyBufferSubData);
    VT_GL_INIT_FUNC(CreateBuffers);
    VT_GL_INIT_FUNC(CreateProgram);
    VT_GL_INIT_FUNC(CreateShader);
    VT_GL_INIT_FUNC(CreateVertexArrays);
    VT_GL_INIT_FUNC(CompileShader);
    VT_GL_INIT_FUNC(DebugMessageCallback);
    VT_GL_INIT_FUNC(DebugMessageControl);
    VT_GL_INIT_FUNC(DeleteBuffers);
    VT_GL_INIT_FUNC(DeleteProgram);
    VT_GL_INIT_FUNC(DeleteShader);
    VT_GL_INIT_FUNC(DeleteVertexArrays);
    VT_GL_INIT_FUNC(DetachShader);
    VT_GL_INIT_FUNC(DrawElementsBaseVertex);
    VT_GL_INIT_FUNC(EnableVertexAttribArray);
    VT_GL_INIT_FUNC(GenBuffers);
    VT_GL_INIT_FUNC(GenVertexArrays);
    VT_GL_INIT_FUNC(GetAttribLocation);
    VT_GL_INIT_FUNC(GetProgramInfoLog);
    VT_GL_INIT_FUNC(GetProgramiv);
    VT_GL_INIT_FUNC(GetShaderInfoLog);
    VT_GL_INIT_FUNC(GetShaderiv);
    VT_GL_INIT_FUNC(GetStringi);
    VT_GL_INIT_FUNC(GetUniformLocation);
    VT_GL_INIT_FUNC(LinkProgram);
    VT_GL_INIT_FUNC(ShaderBinary);
    VT_GL_INIT_FUNC(ShaderSource);
    VT_GL_INIT_FUNC(SpecializeShader);
    VT_GL_INIT_FUNC(Uniform1f);
    VT_GL_INIT_FUNC(Uniform2f);
    VT_GL_INIT_FUNC(Uniform3f);
    VT_GL_INIT_FUNC(Uniform4f);
    VT_GL_INIT_FUNC(Uniform1i);
    VT_GL_INIT_FUNC(UniformMatrix4fv);
    VT_GL_INIT_FUNC(UseProgram);
    VT_GL_INIT_FUNC(ValidateProgram);
    VT_GL_INIT_FUNC(VertexAttribDivisor);
    VT_GL_INIT_FUNC(VertexAttribIPointer);
    VT_GL_INIT_FUNC(VertexAttribPointer);
    #pragma endregion
    static GLboolean initialized = false;

    #ifdef VT_PLATFORM_WINDOWS
    #define ubyte(name) #name
    GLvoid* GetProcAddress(const char* name)
    {
        GLvoid* ptr = (GLvoid*)wglGetProcAddress(name);
        if(ptr == nullptr || (ptr == (GLvoid*)0x1) || (ptr == (GLvoid*)0x2) || (ptr == (GLvoid*)0x3) || (ptr == (GLvoid*)-1))
        {
            HMODULE module = LoadLibraryA("opengl32.dll");
            ptr = (GLvoid*)GetProcAddress(module, name);
        }

        if (!ptr)
        {
            initialized = false;
            VTCoreLogFatal("Failed to Get Address of {} Procedure", name);
        }

        return ptr;
    }
    #elif defined(VT_PLATFORM_LINUX)
    #define ubyte(name) reinterpret_cast<const GLubyte*>(#name)
    GLvoid* GetProcAddress(const GLubyte* name)
    {
        GLvoid* ptr = (GLvoid*)glXGetProcAddress(name);
        if(ptr == nullptr || (ptr == (GLvoid*)0x1) || (ptr == (GLvoid*)0x2) || (ptr == (GLvoid*)0x3) || (ptr == (GLvoid*)-1))
        {
            initialized = false;
        }

        return ptr;
    }
    #endif

    GLboolean LoadOpenGLFunctions()
    {
        if (initialized) return true;
        initialized = true;

        #pragma region loading opengl functions
        #define VT_GL_LOAD(name) gl##name = (PFNGL##name##PROC) GetProcAddress(ubyte(gl##name))
        VT_GL_LOAD(AttachShader);
        VT_GL_LOAD(BindBuffer);
        VT_GL_LOAD(BindSampler);
        VT_GL_LOAD(BindTexture);
        VT_GL_LOAD(BindVertexArray);
        VT_GL_LOAD(BlendEquationSeparate);
        VT_GL_LOAD(BlendFuncSeparate);
        VT_GL_LOAD(BufferData);
        VT_GL_LOAD(BufferSubData);
        VT_GL_LOAD(CopyBufferSubData);
        VT_GL_LOAD(CreateBuffers);
        VT_GL_LOAD(CreateProgram);
        VT_GL_LOAD(CreateShader);
        VT_GL_LOAD(CreateVertexArrays);
        VT_GL_LOAD(CompileShader);
        VT_GL_LOAD(DebugMessageCallback);
        VT_GL_LOAD(DebugMessageControl);
        VT_GL_LOAD(DeleteBuffers);
        VT_GL_LOAD(DeleteProgram);
        VT_GL_LOAD(DeleteShader);
        VT_GL_LOAD(DeleteVertexArrays);
        VT_GL_LOAD(DetachShader);
        VT_GL_LOAD(DrawElementsBaseVertex);
        VT_GL_LOAD(EnableVertexAttribArray);
        VT_GL_LOAD(GenBuffers);
        VT_GL_LOAD(GenVertexArrays);
        VT_GL_LOAD(GetAttribLocation);
        VT_GL_LOAD(GetProgramInfoLog);
        VT_GL_LOAD(GetProgramiv);
        VT_GL_LOAD(GetShaderInfoLog);
        VT_GL_LOAD(GetShaderiv);
        VT_GL_LOAD(GetStringi);
        VT_GL_LOAD(GetUniformLocation);
        VT_GL_LOAD(LinkProgram);
        VT_GL_LOAD(ShaderBinary);
        VT_GL_LOAD(ShaderSource);
        VT_GL_LOAD(SpecializeShader);
        VT_GL_LOAD(Uniform1f);
        VT_GL_LOAD(Uniform2f);
        VT_GL_LOAD(Uniform3f);
        VT_GL_LOAD(Uniform4f);
        VT_GL_LOAD(Uniform1i);
        VT_GL_LOAD(UniformMatrix4fv);
        VT_GL_LOAD(UseProgram);
        VT_GL_LOAD(ValidateProgram);
        VT_GL_LOAD(VertexAttribDivisor);
        VT_GL_LOAD(VertexAttribIPointer);
        VT_GL_LOAD(VertexAttribPointer);
        #pragma endregion

        if (initialized) VTCoreLogTrace("OpenGL Functions Successfully Loaded!\n");
        return initialized;
    }
}
