//
// Created by Vitriol1744 on 27.06.2021.
//
#pragma once

#include "Core/Core.hpp"

#ifdef VT_PLATFORM_WINDOWS
    #include <Windows.h>
    #include "Gl/Gl.h"
    #include "GL/wglext.h"
#elif defined(VT_PLATFORM_LINUX)
    #include <X11/Xlib.h>
    #include <X11/Xutil.h>
    #include <GL/glx.h>
#endif

#include <KHR/khrplatform.h>

#undef APIENTRY
#ifdef VT_PLATFORM_WINDOWS
#define APIENTRY __stdcall
#else
#define APIENTRY
#endif

namespace Vortex::Graphics
{
    #pragma region glTypes
    using GLboolean     = unsigned char;
    using GLbyte        = signed char;
    using GLchar        = char;
    using GLenum        = unsigned int;
    using GLfloat       = float;
    using GLint         = int;
    using GLintptr      = khronos_intptr_t;
    using GLsizei       = int;
    using GLsizeiptr    = khronos_ssize_t;
    using GLuint        = unsigned int;
    using GLvoid        = void;

    using GLDEBUGPROC = GLvoid(APIENTRY*)(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const GLvoid* userParam);
    #pragma endregion

    #pragma region glMacros
    #define GL_VERTEX_ARRAY                 0x8074
    #define GL_MAJOR_VERSION                0x821B
    #define GL_MINOR_VERSION                0x821C
    #define GL_DEBUG_OUTPUT_SYNCHRONOUS     0x8242
    #define GL_DEBUG_SEVERITY_NOTIFICATION  0x826B
    #define GL_ARRAY_BUFFER                 0x8892
    #define GL_ELEMENT_ARRAY_BUFFER         0x8893
    #define GL_STATIC_DRAW                  0x88E4
    #define GL_DYNAMIC_DRAW                 0x88E8
    #define GL_FRAGMENT_SHADER              0x8B30
    #define GL_VERTEX_SHADER                0x8B31
    #define GL_BOOL                         0x8B56
    #define GL_COMPILE_STATUS               0x8B81
    #define GL_LINK_STATUS                  0x8B82
    #define GL_INFO_LOG_LENGTH              0x8B84
    #define GL_COPY_READ_BUFFER             0x8F36
    #define GL_COPY_WRITE_BUFFER            0x8F37
    #define GL_DEBUG_SEVERITY_HIGH          0x9146
    #define GL_DEBUG_SEVERITY_MEDIUM        0x9147
    #define GL_DEBUG_SEVERITY_LOW           0x9148
    #define GL_DEBUG_OUTPUT                 0x92E0
    #pragma endregion

    #pragma region glFunctions
    #define VT_DECLARE_GL_FUNC(name, return_type, ...) \
        using PFNGL##name##PROC = return_type(APIENTRY*)(__VA_ARGS__); \
        extern PFNGL##name##PROC gl##name
    VT_DECLARE_GL_FUNC(AttachShader, GLvoid, GLuint program, GLuint shader);
    VT_DECLARE_GL_FUNC(BindBuffer, GLvoid, GLenum target, GLuint buffer);
    VT_DECLARE_GL_FUNC(BindSampler, GLvoid, GLuint unit, GLuint sampler);
    VT_DECLARE_GL_FUNC(BindTexture, GLvoid, GLenum target, GLuint texture);
    VT_DECLARE_GL_FUNC(BindVertexArray, GLvoid, GLuint array);
    VT_DECLARE_GL_FUNC(BlendEquationSeparate, GLvoid, GLenum modeRGB, GLenum modeAlpha);
    VT_DECLARE_GL_FUNC(BlendFuncSeparate, GLvoid, GLenum srcRGB, GLenum dstRGB, GLenum srcAlpha, GLenum dstAlpha);
    VT_DECLARE_GL_FUNC(BufferData, GLvoid, GLenum target, GLsizeiptr size, const GLvoid* data, GLenum usage);
    VT_DECLARE_GL_FUNC(BufferSubData, GLvoid, GLenum target, GLintptr offset, const GLsizeiptr size, const GLvoid* data);
    VT_DECLARE_GL_FUNC(CopyBufferSubData, GLvoid, GLenum readTarget, GLenum writeTarget, GLintptr readOffset, GLintptr writeOffset, GLsizeiptr size);
    VT_DECLARE_GL_FUNC(CreateBuffers, GLvoid, GLsizei n, GLuint* buffers);
    VT_DECLARE_GL_FUNC(CreateProgram, GLuint);
    VT_DECLARE_GL_FUNC(CreateShader, GLuint, GLenum shaderType);
    VT_DECLARE_GL_FUNC(CreateVertexArrays, GLvoid, GLsizei n, GLuint* arrays);
    VT_DECLARE_GL_FUNC(CompileShader, GLvoid, GLuint shader);
    VT_DECLARE_GL_FUNC(DebugMessageCallback, GLvoid, GLDEBUGPROC callback, GLvoid* userParam);
    VT_DECLARE_GL_FUNC(DebugMessageControl, GLvoid, GLenum source, GLenum type, GLenum severity, GLsizei count, const GLuint* ids, GLboolean enabled);
    VT_DECLARE_GL_FUNC(DeleteBuffers, GLvoid, GLsizei n, const GLuint* buffers);
    VT_DECLARE_GL_FUNC(DeleteProgram, GLvoid, GLuint program);
    VT_DECLARE_GL_FUNC(DeleteShader, GLvoid, GLuint shader);
    VT_DECLARE_GL_FUNC(DeleteVertexArrays, GLvoid, GLsizei n, const GLuint* arrays);
    VT_DECLARE_GL_FUNC(DetachShader, GLvoid, GLuint program, GLuint shader);
    VT_DECLARE_GL_FUNC(DrawElementsBaseVertex, GLvoid, GLenum mode, GLsizei count, GLenum type, GLvoid* indices, GLint baseVertex);
    VT_DECLARE_GL_FUNC(EnableVertexAttribArray, GLvoid, GLuint index);
    VT_DECLARE_GL_FUNC(GenBuffers, GLvoid, GLsizei n, GLuint* buffers);
    VT_DECLARE_GL_FUNC(GenVertexArrays, GLvoid, GLsizei n, GLuint* arrays);
    VT_DECLARE_GL_FUNC(GetAttribLocation, GLint, GLuint program, const GLchar* name);
    VT_DECLARE_GL_FUNC(GetProgramInfoLog, GLvoid, GLuint program, GLsizei maxLength, GLsizei* length, GLchar* infoLog);
    VT_DECLARE_GL_FUNC(GetProgramiv, GLvoid, GLuint program, GLenum pname, GLint* params);
    VT_DECLARE_GL_FUNC(GetShaderInfoLog, GLvoid, GLuint shader, GLsizei maxLength, GLsizei* length, GLchar* infoLog);
    VT_DECLARE_GL_FUNC(GetShaderiv, GLvoid, GLuint shader, GLenum pname, GLint* params);
    VT_DECLARE_GL_FUNC(GetStringi, GLubyte*, GLenum name, GLuint index);
    VT_DECLARE_GL_FUNC(GetUniformLocation, GLint, GLuint program, const GLchar* name);
    VT_DECLARE_GL_FUNC(LinkProgram, GLvoid, GLuint program);
    VT_DECLARE_GL_FUNC(ShaderBinary, GLvoid, GLsizei count, const GLuint* shaders, GLenum binaryFormat, const GLvoid* binary, GLsizei length);
    VT_DECLARE_GL_FUNC(ShaderSource, GLvoid, GLuint shader, GLsizei count, const GLchar** string, const GLint* length);
    VT_DECLARE_GL_FUNC(SpecializeShader, GLvoid, GLuint shader, const GLchar* entryPoint, GLuint numSpecializationConstants, const GLuint* constantIndex, const GLuint* pConstantValue);
    VT_DECLARE_GL_FUNC(Uniform1f, GLvoid, GLint location, GLfloat v0);
    VT_DECLARE_GL_FUNC(Uniform2f, GLvoid, GLint location, GLfloat v0, GLfloat v1);
    VT_DECLARE_GL_FUNC(Uniform3f, GLvoid, GLint location, GLfloat v0, GLfloat v1, GLfloat v2);
    VT_DECLARE_GL_FUNC(Uniform4f, GLvoid, GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3);
    VT_DECLARE_GL_FUNC(Uniform1i, GLvoid, GLint location, GLuint v0);
    VT_DECLARE_GL_FUNC(UniformMatrix4fv, GLvoid, GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);
    VT_DECLARE_GL_FUNC(UseProgram, GLvoid, GLuint program);
    VT_DECLARE_GL_FUNC(ValidateProgram, GLvoid, GLuint program);
    VT_DECLARE_GL_FUNC(VertexAttribDivisor, GLvoid, GLuint index, GLuint divisor);
    VT_DECLARE_GL_FUNC(VertexAttribIPointer, GLvoid, GLuint index, GLint size, GLenum type, GLsizei stride, const GLvoid* pointer);
    VT_DECLARE_GL_FUNC(VertexAttribPointer, GLvoid, GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const GLvoid* pointer);
    #pragma endregion

    #ifdef VT_PLATFORM_WINDOWS
    GLvoid* GetProcAddress(const GLbyte* name);
    #elif defined(VT_PLATFORM_LINUX)
    GLvoid* GetProcAddress(const GLubyte* name);
    #endif

    GLboolean LoadOpenGLFunctions();
}