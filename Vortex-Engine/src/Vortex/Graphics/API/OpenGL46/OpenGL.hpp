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
    using GLboolean     = bool;
    using GLbyte        = signed char;
    using GLchar        = char;
    using GLenum        = uint32;
    using GLfloat       = float;
    using GLint         = int32;
    using GLintptr      = khronos_intptr_t;
    using GLsizei       = int32;
    using GLsizeiptr    = khronos_ssize_t;
    using GLuint        = uint32;
    using GLvoid        = void;

    using GLDEBUGPROC = void (APIENTRY*)(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam);
    #pragma endregion

    #pragma region glMacros
    #define GL_VERTEX_ARRAY                 0x8074
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
    using PFNGLATTACHSHADERPROC = GLvoid(*)(GLuint program, GLuint shader);
    extern PFNGLATTACHSHADERPROC glAttachShader;

    using PFNGLBINDBUFFERPROC = GLvoid(*)(GLenum target, GLuint buffer);
    extern PFNGLBINDBUFFERPROC glBindBuffer;

    using PFNGLBINDVERTEXARRAYPROC = GLvoid(*)(GLuint array);
    extern PFNGLBINDVERTEXARRAYPROC glBindVertexArray;

    using PFNGLBUFFERDATAPROC = GLvoid(*)(GLenum target, GLsizeiptr size, const GLvoid* data, GLenum usage);
    extern PFNGLBUFFERDATAPROC glBufferData;

    using PFNGLBUFFERSUBDATAPROC = GLvoid(*)(GLenum target, GLintptr offset, const GLsizeiptr size, const GLvoid* data);
    extern PFNGLBUFFERSUBDATAPROC glBufferSubData;

    using PFNGLCOPYBUFFERSUBDATAPROC = GLvoid(*)(GLenum readTarget, GLenum writeTarget, GLintptr readOffset, GLintptr writeOffset, GLsizeiptr size);
    extern PFNGLCOPYBUFFERSUBDATAPROC glCopyBufferSubData;

    using PFNGLCREATEBUFFERSPROC = GLvoid(*)(GLsizei n, GLuint* buffers);
    extern PFNGLCREATEBUFFERSPROC glCreateBuffers;

    using PFNGLCREATEPROGRAMPROC = GLuint(*)();
    extern PFNGLCREATEPROGRAMPROC glCreateProgram;

    using PFNGLCREATESHADERPROC = GLuint(*)(GLenum shaderType);
    extern PFNGLCREATESHADERPROC glCreateShader;

    using PFNGLCREATEVERTEXARRAYSPROC = GLvoid(*)(GLsizei n, GLuint* arrays);
    extern PFNGLCREATEVERTEXARRAYSPROC glCreateVertexArrays;

    using PFNGLCOMPILESHADERPROC = GLvoid(*)(GLuint shader);
    extern PFNGLCOMPILESHADERPROC glCompileShader;

    using PFNGLDEBUGMESSAGECALLBACKPROC = GLvoid(*)(GLDEBUGPROC callback, GLvoid* userParam);
    extern PFNGLDEBUGMESSAGECALLBACKPROC glDebugMessageCallback;

    using PFNGLDEBUGMESSAGECONTROLPROC = GLvoid(*)(GLenum source, GLenum type, GLenum severity, GLsizei count, const GLuint* ids, GLboolean enabled);
    extern PFNGLDEBUGMESSAGECONTROLPROC glDebugMessageControl;

    using PFNGLDELETEBUFFERSPROC = GLvoid(*)(GLsizei n, const GLuint* buffers);
    extern PFNGLDELETEBUFFERSPROC glDeleteBuffers;

    using PFNGLDELETEPROGRAMPROC = GLvoid(*)(GLuint program);
    extern PFNGLDELETEPROGRAMPROC glDeleteProgram;

    using PFNGLDELETESHADERPROC = GLvoid(*)(GLuint shader);
    extern PFNGLDELETESHADERPROC glDeleteShader;

    using PFNGLDETACHSHADERPROC = GLvoid(*)(GLuint program, GLuint shader);
    extern PFNGLDETACHSHADERPROC glDetachShader;

    using PFNGLENABLEVERTEXATTRIBARRAYPROC = GLvoid(*)(GLuint index);
    extern PFNGLENABLEVERTEXATTRIBARRAYPROC glEnableVertexAttribArray;

    using PFNGLGETPROGRAMINFOLOGPROC = GLvoid(*)(GLuint program, GLsizei maxLength, GLsizei* length, GLchar* infoLog);
    extern PFNGLGETPROGRAMINFOLOGPROC glGetProgramInfoLog;

    using PFNGLGETPROGRAMIVPROC = GLvoid(*)(GLuint program, GLenum pname, GLint* params);
    extern PFNGLGETPROGRAMIVPROC glGetProgramiv;

    using PFNGLGETSHADERINFOLOG = GLvoid(*)(GLuint shader, GLsizei maxLength, GLsizei* length, GLchar* infoLog);
    extern PFNGLGETSHADERINFOLOG glGetShaderInfoLog;

    using PFNGLGETSHADERIVPROC = GLvoid(*)(GLuint shader, GLenum pname, GLint* params);
    extern PFNGLGETSHADERIVPROC glGetShaderiv;

    using PFNGLGETUNIFORMLOCATIONPROC = GLint(*)(GLuint program, const GLchar* name);
    extern PFNGLGETUNIFORMLOCATIONPROC glGetUniformLocation;

    using PFNGLLINKPROGRAMPROC = GLvoid(*)(GLuint program);
    extern PFNGLLINKPROGRAMPROC glLinkProgram;

    using PFNGLSHADERBINARYPROC = GLvoid(*)(GLsizei count, const GLuint* shaders, GLenum binaryFormat, const GLvoid* binary, GLsizei length);
    extern PFNGLSHADERBINARYPROC glShaderBinary;

    using PFNGLSHADERSOURCEPROC = GLvoid(*)(GLuint shader, GLsizei count, const GLchar** string, const GLint* length);
    extern PFNGLSHADERSOURCEPROC glShaderSource;

    using PFNGLSPECIALIZESHADERPROC = GLvoid(*)(GLuint shader, const GLchar* entryPoint, GLuint numSpecializationConstants, const GLuint* constantIndex, const GLuint* pConstantValue);
    extern PFNGLSPECIALIZESHADERPROC glSpecializeShader;

    using PFNGLUNIFORM1FPROC = GLvoid(*)(GLint location, GLfloat v0);
    extern PFNGLUNIFORM1FPROC glUniform1f;

    using PFNGLUNIFORM2FPROC = GLvoid(*)(GLint location, GLfloat v0, GLfloat v1);
    extern PFNGLUNIFORM2FPROC glUniform2f;

    using PFNGLUNIFORM3FPROC = GLvoid(*)(GLint location, GLfloat v0, GLfloat v1, GLfloat v2);
    extern PFNGLUNIFORM3FPROC glUniform3f;

    using PFNGLUNIFORM4FPROC = GLvoid(*)(GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3);
    extern PFNGLUNIFORM4FPROC glUniform4f;

    using PFNGLUNIFORMMATRIX4FVPROC = GLvoid(*)(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);
    extern PFNGLUNIFORMMATRIX4FVPROC glUniformMatrix4fv;

    using PFNGLUSEPROGRAMPROC = GLvoid(*)(GLuint program);
    extern PFNGLUSEPROGRAMPROC glUseProgram;

    using PFNGLVALIDATEPROGRAMPROC = GLvoid(*)(GLuint program);
    extern PFNGLVALIDATEPROGRAMPROC glValidateProgram;

    using PFNGLVERTEXATTRIBDIVISORPROC = GLvoid(*)(GLuint index, GLuint divisor);
    extern PFNGLVERTEXATTRIBDIVISORPROC glVertexAttribDivisor;

    using PFNGLVERTEXATTRIBIPOINTERPROC = GLvoid(*)(GLuint index, GLint size, GLenum type, GLsizei stride, const GLvoid* pointer);
    extern PFNGLVERTEXATTRIBIPOINTERPROC glVertexAttribIPointer;

    using PFNGLVERTEXATTRIBPOINTERPROC = GLvoid(*)(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const GLvoid* pointer);
    extern PFNGLVERTEXATTRIBPOINTERPROC glVertexAttribPointer;
    #pragma endregion

    #ifdef VT_PLATFORM_WINDOWS
    void* GetProcAddress(const GLbyte* name);
    #elif defined(VT_PLATFORM_LINUX)
    void* GetProcAddress(const GLubyte* name);
    #endif

    bool LoadGLFunctions();
}