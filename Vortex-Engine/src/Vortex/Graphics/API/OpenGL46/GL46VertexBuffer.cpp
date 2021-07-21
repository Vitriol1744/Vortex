//
// Created by Vitriol1744 on 27.06.2021.
//
#include "GL46VertexBuffer.hpp"

namespace Vortex::Graphics
{
    GL46VertexBuffer::GL46VertexBuffer() noexcept : GL46VertexBuffer(0) { }

    GL46VertexBuffer::GL46VertexBuffer(GLuint size)
    {
        glCreateBuffers(1, &id);
        glBindBuffer(GL_ARRAY_BUFFER, id);
        glBufferData(GL_ARRAY_BUFFER, size, nullptr, GL_DYNAMIC_DRAW);
    }

    GL46VertexBuffer::GL46VertexBuffer(GLvoid* vertices, GLuint size)
        : size(size)
    {
        glCreateBuffers(1, &id);
        glBindBuffer(GL_ARRAY_BUFFER, id);
        glBufferData(GL_ARRAY_BUFFER, size, vertices, GL_STATIC_DRAW);
    }

    GL46VertexBuffer::GL46VertexBuffer(const GL46VertexBuffer& vertexBuffer)
    {
        glCreateBuffers(1, &id);
        glBindBuffer(GL_COPY_READ_BUFFER, vertexBuffer.GetID());
        glBindBuffer(GL_COPY_WRITE_BUFFER, id);

        glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER, 0, 0, vertexBuffer.GetSize());

        glBindBuffer(GL_COPY_READ_BUFFER, 0);
        glBindBuffer(GL_COPY_WRITE_BUFFER, 0);
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

    GLvoid GL46VertexBuffer::SetData(GLvoid* vertices, GLuint size) const
    {
        Bind();
        glBufferData(GL_ARRAY_BUFFER, size, vertices, GL_STATIC_DRAW);
    }
}