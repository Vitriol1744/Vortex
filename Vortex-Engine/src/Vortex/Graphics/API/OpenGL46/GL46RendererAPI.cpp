//
// Created by Vitriol1744 on 26.06.2021.
//
#include "vtpch.hpp"
#include "GL46RendererAPI.hpp"

namespace Vortex::Graphics
{
    GLvoid GL46RendererAPI::SetClearColorImpl(Math::Vec4 color) noexcept
    {
        glClearColor(color.r, color.g, color.b, color.a);
    }

    GLvoid GL46RendererAPI::ClearImpl() noexcept
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }

    GLvoid GL46RendererAPI::DrawIndexedImpl(const Ref<IVertexArray> mesh, uint32_t indicesCount)
    {
         mesh->Bind();
         glDrawElements(GL_TRIANGLES, indicesCount, GL_UNSIGNED_INT, nullptr);
    }

    GLvoid GL46RendererAPI::Initialize()
    {
        glEnable(GL_DEBUG_OUTPUT);
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);

        glDebugMessageCallback(ErrorCallback, nullptr);

        glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_NOTIFICATION, 0, nullptr, GL_FALSE);
    }

    GLvoid GL46RendererAPI::ErrorCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const GLvoid* userParam)
    {
        switch (severity)
        {
            case GL_DEBUG_SEVERITY_HIGH:
                VT_CORE_LOG_FATAL("OpenGL Error Callback: {}", message);
                break;
            case GL_DEBUG_SEVERITY_MEDIUM:
                VT_CORE_LOG_ERROR("OpenGL Error Callback: {}", message);
                break;
            case GL_DEBUG_SEVERITY_LOW:
                VT_CORE_LOG_WARN("OpenGL Error Callback: {}", message);
                break;
            case GL_DEBUG_SEVERITY_NOTIFICATION:
                VT_CORE_LOG_TRACE("OpenGL Error Callback: {}", message);
                break;
        }
    }
}