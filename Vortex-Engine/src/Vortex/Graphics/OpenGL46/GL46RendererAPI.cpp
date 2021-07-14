//
// Created by Vitriol1744 on 26.06.2021.
//
#include "vtpch.hpp"
#include "GL46RendererAPI.hpp"

namespace Vortex::Graphics
{
    GLvoid GL46RendererAPI::SetClearColor(Math::Vec4 color) noexcept
    {
        glClearColor(color.r, color.g, color.b, color.a);
    }

    GLvoid GL46RendererAPI::Clear() noexcept
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    }

    GLvoid GL46RendererAPI::DrawIndexed(const Ref<IMesh>& mesh, uint32_t indicesCount)
    {
         mesh->Bind();
         glDrawElements(GL_TRIANGLES, indicesCount, GL_UNSIGNED_INT, nullptr);
    }
}