//
// Created by vitriol1744 on 11.07.2021.
//
#include "OpenGL.hpp"

namespace Vortex::Graphics
{
    #pragma region glFunctions
    PFNGLATTACHSHADERPROC               glAttachShader              = nullptr;
    PFNGLBINDBUFFERPROC                 glBindBuffer                = nullptr;
    PFNGLBINDVERTEXARRAYPROC            glBindVertexArray           = nullptr;
    PFNGLBUFFERDATAPROC                 glBufferData                = nullptr;
    PFNGLCREATEBUFFERSPROC              glCreateBuffers             = nullptr;
    PFNGLCREATEPROGRAMPROC              glCreateProgram             = nullptr;
    PFNGLCREATESHADERPROC               glCreateShader              = nullptr;
    PFNGLCREATEVERTEXARRAYSPROC         glCreateVertexArrays        = nullptr;
    PFNGLCOMPILESHADERPROC              glCompileShader             = nullptr;
    PFNGLDELETEBUFFERSPROC              glDeleteBuffers             = nullptr;
    PFNGLDELETEPROGRAMPROC              glDeleteProgram             = nullptr;
    PFNGLDELETESHADERPROC               glDeleteShader              = nullptr;
    PFNGLDETACHSHADERPROC               glDetachShader              = nullptr;
    PFNGLENABLEVERTEXATTRIBARRAYPROC    glEnableVertexAttribArray   = nullptr;
    PFNGLGETPROGRAMINFOLOGPROC          glGetProgramInfoLog         = nullptr;
    PFNGLGETPROGRAMIVPROC               glGetProgramiv              = nullptr;
    PFNGLGETSHADERINFOLOG               glGetShaderInfoLog          = nullptr;
    PFNGLGETSHADERIVPROC                glGetShaderiv               = nullptr;
    PFNGLGETUNIFORMLOCATIONPROC         glGetUniformLocation        = nullptr;
    PFNGLLINKPROGRAMPROC                glLinkProgram               = nullptr;
    PFNGLSHADERBINARYPROC               glShaderBinary              = nullptr;
    PFNGLSHADERSOURCEPROC               glShaderSource              = nullptr;
    PFNGLSPECIALIZESHADERPROC           glSpecializeShader          = nullptr;
    PFNGLUNIFORM1FPROC                  glUniform1f                 = nullptr;
    PFNGLUNIFORM2FPROC                  glUniform2f                 = nullptr;
    PFNGLUNIFORM3FPROC                  glUniform3f                 = nullptr;
    PFNGLUNIFORM4FPROC                  glUniform4f                 = nullptr;
    PFNGLUNIFORMMATRIX4FVPROC           glUniformMatrix4fv          = nullptr;
    PFNGLUSEPROGRAMPROC                 glUseProgram                = nullptr;
    PFNGLVALIDATEPROGRAMPROC            glValidateProgram           = nullptr;
    PFNGLVERTEXATTRIBDIVISORPROC        glVertexAttribDivisor       = nullptr;
    PFNGLVERTEXATTRIBIPOINTERPROC       glVertexAttribIPointer      = nullptr;
    PFNGLVERTEXATTRIBPOINTERPROC        glVertexAttribPointer       = nullptr;
    #pragma endregion
    static bool initialized = false;

    #ifdef VT_PLATFORM_WINDOWS
    #define ubyte(name)
    void* GetProcAddress(const char* name)
    {
        void* p = (void*)wglGetProcAddress(name);
        if(p == 0 || (p == (void*)0x1) || (p == (void*)0x2) || (p == (void*)0x3) || (p == (void*)-1))
        {
            HMODULE module = LoadLibraryA("opengl32.dll");
            p = (void *)GetProcAddress(module, name);
        }

        if (!p) initialized = false;
        return p;
    }
    #elif defined(VT_PLATFORM_LINUX)
    #define ubyte(name) reinterpret_cast<const unsigned char *>(name)
    void* GetProcAddress(const unsigned char* name)
    {
        void* p = (void*)glXGetProcAddress(name);
        if(p == nullptr || (p == (void*)0x1) || (p == (void*)0x2) || (p == (void*)0x3) || (p == (void*)-1))
        {
            initialized = false;
        }

        return p;
    }
    #endif

    uint32 vao;

    bool LoadGLFunctions()
    {
        if (initialized) return true;
        initialized = true;

        glAttachShader              = (PFNGLATTACHSHADERPROC)               GetProcAddress(ubyte("glAttachShader"               ));
        glBindBuffer                = (PFNGLBINDBUFFERPROC)                 GetProcAddress(ubyte("glBindBuffer"                 ));
        glBindVertexArray           = (PFNGLBINDVERTEXARRAYPROC)            GetProcAddress(ubyte("glBindVertexArray"            ));
        glBufferData                = (PFNGLBUFFERDATAPROC)                 GetProcAddress(ubyte("glBufferData"                 ));
        glCreateBuffers             = (PFNGLCREATEBUFFERSPROC)              GetProcAddress(ubyte("glCreateBuffers"              ));
        glCreateProgram             = (PFNGLCREATEPROGRAMPROC)              GetProcAddress(ubyte("glCreateProgram"              ));
        glCreateShader              = (PFNGLCREATESHADERPROC)               GetProcAddress(ubyte("glCreateShader"               ));
        glCreateVertexArrays        = (PFNGLCREATEVERTEXARRAYSPROC)         GetProcAddress(ubyte("glCreateVertexArrays"         ));
        glCompileShader             = (PFNGLCOMPILESHADERPROC)              GetProcAddress(ubyte("glCompileShader"              ));
        glDeleteBuffers             = (PFNGLDELETEBUFFERSPROC)              GetProcAddress(ubyte("glDeleteBuffers"              ));
        glDeleteProgram             = (PFNGLDELETEPROGRAMPROC)              GetProcAddress(ubyte("glDeleteProgram"              ));
        glDeleteShader              = (PFNGLDELETESHADERPROC)               GetProcAddress(ubyte("glDeleteShaders"              ));
        glDetachShader              = (PFNGLDETACHSHADERPROC)               GetProcAddress(ubyte("glDetachShader"               ));
        glEnableVertexAttribArray   = (PFNGLENABLEVERTEXATTRIBARRAYPROC)    GetProcAddress(ubyte("glEnableVertexAttribArray"    ));
        glGetProgramInfoLog         = (PFNGLGETPROGRAMINFOLOGPROC)          GetProcAddress(ubyte("glGetProgramInfoLog"          ));
        glGetProgramiv              = (PFNGLGETPROGRAMIVPROC)               GetProcAddress(ubyte("glGetProgramiv"               ));
        glGetShaderInfoLog          = (PFNGLGETSHADERINFOLOG)               GetProcAddress(ubyte("glGetShaderInfoLog"           ));
        glGetShaderiv               = (PFNGLGETSHADERIVPROC)                GetProcAddress(ubyte("glGetShaderiv"                ));
        glGetUniformLocation        = (PFNGLGETUNIFORMLOCATIONPROC)         GetProcAddress(ubyte("glGetUniformLocation"         ));
        glLinkProgram               = (PFNGLLINKPROGRAMPROC)                GetProcAddress(ubyte("glLinkProgram"                ));
        glShaderBinary              = (PFNGLSHADERBINARYPROC)               GetProcAddress(ubyte("glShaderBinary"               ));
        glShaderSource              = (PFNGLSHADERSOURCEPROC)               GetProcAddress(ubyte("glShaderSource"               ));
        glSpecializeShader          = (PFNGLSPECIALIZESHADERPROC)           GetProcAddress(ubyte("glSpecializeShader"           ));
        glUniform1f                 = (PFNGLUNIFORM1FPROC)                  GetProcAddress(ubyte("glUniform1f"                  ));
        glUniform2f                 = (PFNGLUNIFORM2FPROC)                  GetProcAddress(ubyte("glUniform2f"                  ));
        glUniform3f                 = (PFNGLUNIFORM3FPROC)                  GetProcAddress(ubyte("glUniform3f"                  ));
        glUniform4f                 = (PFNGLUNIFORM4FPROC)                  GetProcAddress(ubyte("glUniform4f"                  ));
        glUniformMatrix4fv          = (PFNGLUNIFORMMATRIX4FVPROC)           GetProcAddress(ubyte("glUniformMatrix4fv"           ));
        glUseProgram                = (PFNGLUSEPROGRAMPROC)                 GetProcAddress(ubyte("glUseProgram"                 ));
        glValidateProgram           = (PFNGLVALIDATEPROGRAMPROC)            GetProcAddress(ubyte("glValidateProgram"            ));
        glVertexAttribDivisor       = (PFNGLVERTEXATTRIBDIVISORPROC)        GetProcAddress(ubyte("glVertexAttribDivisor"        ));
        glVertexAttribIPointer      = (PFNGLVERTEXATTRIBIPOINTERPROC)       GetProcAddress(ubyte("glVertexAttribIPointer"       ));
        glVertexAttribPointer       = (PFNGLVERTEXATTRIBPOINTERPROC)        GetProcAddress(ubyte("glVertexAttribPointer"        ));

        VT_CORE_LOG_TRACE("GL Functions Loaded!\n");
        return initialized;
    }
}
