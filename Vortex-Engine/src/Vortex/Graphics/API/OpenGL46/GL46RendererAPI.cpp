//
// Created by Vitriol1744 on 26.06.2021.
//
#include "vtpch.hpp"
#include "GL46RendererAPI.hpp"

namespace Vortex::Graphics
{
    using namespace Math;

    GLvoid GL46RendererAPI::InitializeImpl()
    {
        
    }

    GLvoid GL46RendererAPI::SetClearColorImpl(Math::Vec4 color)
    {
        glClearColor(color.r, color.g, color.b, color.a);
    }
    GLvoid GL46RendererAPI::SetViewportImpl(const Rectangle& rect)
    {
        glViewport(rect.x, rect.y, rect.width, rect.height);
    }

    GLvoid GL46RendererAPI::ClearImpl() noexcept
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }
    GLvoid GL46RendererAPI::DrawIndexedImpl(const Ref<IVertexArray>& mesh, uint32_t indicesCount)
    {
         mesh->Bind();
         glDrawElements(GL_TRIANGLES, indicesCount, GL_UNSIGNED_INT, nullptr);
    }

    GLboolean GL46RendererAPI::PostInitialize()
    {
        if (!LoadOpenGLFunctions())
        {
            VTCoreLogError("Failed to Load OpenGL Functions!");
            VTCoreLogError("Failed to Initialize OpenGL!");
            return false;
        }

        VTCoreLogTrace("OpenGL Initialized!");
        VTCoreLogInfo("Version: {}", glGetString(GL_VERSION));
        VTCoreLogInfo("Vendor: {}", glGetString(GL_VENDOR));
        VTCoreLogInfo("Renderer: {}", glGetString(GL_RENDERER));
        VTCoreLogInfo("Shading Language Version: {}", glGetString(GL_SHADING_LANGUAGE_VERSION));

        glEnable(GL_DEBUG_OUTPUT);
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);

        glDebugMessageCallback(ErrorCallback, nullptr);
        glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_NOTIFICATION, 0, nullptr, GL_FALSE);

        glEnable(GL_DEPTH_TEST);

        return true;
    }

    GLvoid GL46RendererAPI::ErrorCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const GLvoid* userParam)
    {
        switch (severity)
        {
            case GL_DEBUG_SEVERITY_HIGH:
                VTCoreLogFatal("OpenGL Error Callback: {}", message);
                break;
            case GL_DEBUG_SEVERITY_MEDIUM:
                VTCoreLogError("OpenGL Error Callback: {}", message);
                break;
            case GL_DEBUG_SEVERITY_LOW:
                VTCoreLogWarn("OpenGL Error Callback: {}", message);
                break;
            case GL_DEBUG_SEVERITY_NOTIFICATION:
                VTCoreLogTrace("OpenGL Error Callback: {}", message);
                break;
        }
    }
}