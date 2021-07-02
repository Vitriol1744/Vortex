//
// Created by Vitriol1744 on 26.06.2021.
//
#include "vtpch.hpp"
#include "GL46RendererAPI.hpp"

#include "OpenGL.hpp"

namespace Vortex
{
    void GL46RendererAPI::SetClearColor(Vec4 color)
    {
        glClearColor(color.r, color.g, color.b, color.a);
    }

    void GL46RendererAPI::Clear()
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }
}