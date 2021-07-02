//
// Created by Vitriol1744 on 27.06.2021.
//
#include "vtpch.hpp"
#include "GL46VertexBuffer.hpp"

#include "OpenGL.hpp"

namespace Vortex
{
    GL46VertexBuffer::GL46VertexBuffer(float* vertices, uint32 size)
        : id(0)
    {
        glCreateBuffers(1, &id);
        glBindBuffer(GL_ARRAY_BUFFER, id);
        glBufferData(GL_ARRAY_BUFFER, size, vertices, GL_STATIC_DRAW);
    }

    GL46VertexBuffer::~GL46VertexBuffer()
    {
        glDeleteBuffers(1, &id);
    }

    void GL46VertexBuffer::Bind()
    {
        glBindBuffer(GL_ARRAY_BUFFER, id);
    }

    void GL46VertexBuffer::Unbind()
    {
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }
}