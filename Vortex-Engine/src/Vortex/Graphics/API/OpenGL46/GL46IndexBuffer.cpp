//
// Created by vitriol1744 on 03.07.2021.
//
#include "GL46IndexBuffer.hpp"

namespace Vortex::Graphics
{
    GL46IndexBuffer::GL46IndexBuffer() noexcept : GL46IndexBuffer(0) { }
    GL46IndexBuffer::GL46IndexBuffer(GLuint size) noexcept
    {
        glCreateBuffers(1, &id);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, id);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, size, nullptr, GL_DYNAMIC_DRAW);
    }

    GL46IndexBuffer::GL46IndexBuffer(GLuint* indices, GLuint size)
    {
        glCreateBuffers(1, &id);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, id);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, size, indices, GL_STATIC_DRAW);

        count = size / sizeof(GLuint);
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

    GLvoid GL46IndexBuffer::Data(GLuint* indices, GLuint size)
    {
        Bind();
        glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, size, indices);
        count = size / sizeof(GLuint);
    }
}