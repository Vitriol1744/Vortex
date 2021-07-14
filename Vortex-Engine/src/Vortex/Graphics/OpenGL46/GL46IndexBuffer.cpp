//
// Created by vitriol1744 on 03.07.2021.
//
#include "GL46IndexBuffer.hpp"

namespace Vortex::Graphics
{
    GL46IndexBuffer::GL46IndexBuffer() noexcept
    {
        glCreateBuffers(1, &id);
    }

    GL46IndexBuffer::GL46IndexBuffer(GLuint* indices, GLuint size)
    {
        glCreateBuffers(1, &id);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, id);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, size, indices, GL_STATIC_DRAW);
    }
    
    GL46IndexBuffer::~GL46IndexBuffer() noexcept
    {
        glDeleteBuffers(1, &id);
    }

    GLvoid GL46IndexBuffer::Bind() const noexcept
    {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, id);
    }

    GLvoid GL46IndexBuffer::Unbind() const noexcept
    {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    }

    GLvoid GL46IndexBuffer::SetData(GLuint* indices, GLuint size) const
    {
        Bind();
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, size, indices, GL_DYNAMIC_DRAW);
    }
}