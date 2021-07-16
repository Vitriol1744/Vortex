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

namespace Vortex::Graphics
{
    #pragma region glTypes
    using GLenum        = uint32;
    using GLint         = int32;
    using GLsizei       = int32;
    using GLsizeiptr    = khronos_ssize_t;
    using GLuint        = uint32;
    using GLvoid        = void;
    #pragma endregion

    #pragma region glMacros
    #define GL_ARRAY_BUFFER 0x8892
    #define GL_STATIC_DRAW 0x88E4
    #pragma endregion

    #pragma region glFunctions
    using PFNGLATTACHSHADERPROC = GLvoid(*)(GLuint program, GLuint shader);
    extern PFNGLATTACHSHADERPROC glAttachShader;

    using PFNGLBINDBUFFERPROC = GLvoid(*)(GLenum target, GLuint buffer);
    extern PFNGLBINDBUFFERPROC glBindBuffer;

    using PFNGLBINDVERTEXARRAYPROC = GLvoid(*)(GLuint array);
    extern PFNGLBINDVERTEXARRAYPROC glBindVertexArray;

    using PFNGLBUFFERDATAPROC = GLvoid(*)(GLenum target, GLsizeiptr size, const GLvoid * data, GLenum usage);
    extern PFNGLBUFFERDATAPROC glBufferData;

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
    void* GetProcAddress(const char* name);
    #elif defined(VT_PLATFORM_LINUX)
    void* GetProcAddress(const unsigned char* name);
    #endif

    bool LoadGLFunctions();
}