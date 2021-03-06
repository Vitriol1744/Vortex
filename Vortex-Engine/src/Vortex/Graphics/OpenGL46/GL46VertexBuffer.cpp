//
// Created by Vitriol1744 on 27.06.2021.
//
#include "GL46VertexBuffer.hpp"

namespace Vortex::Graphics
{
    GL46VertexBuffer::GL46VertexBuffer(GLfloat* vertices, GLuint size)
    {
        glCreateBuffers(1, &id);
        glBindBuffer(GL_ARRAY_BUFFER, id);
        glBufferData(GL_ARRAY_BUFFER, size, vertices, GL_STATIC_DRAW);
    }

    GL46VertexBuffer::~GL46VertexBuffer() noexcept
    {
        glDeleteBuffers(1, &id);
    }

    GLvoid GL46VertexBuffer::Bind() const noexcept
    {
        glBindBuffer(GL_ARRAY_BUFFER, id);
    }

    GLvoid GL46VertexBuffer::Unbind() const noexcept
    {
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

    GLvoid GL46VertexBuffer::SetData(GLuint* vertices, GLuint size) const
    {
        Bind();
        glBufferData(GL_ARRAY_BUFFER, size, vertices, GL_DYNAMIC_DRAW);
    }
}