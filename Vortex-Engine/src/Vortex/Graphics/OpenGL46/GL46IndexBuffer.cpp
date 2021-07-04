//
// Created by vitriol1744 on 03.07.2021.
//
#include "GL46IndexBuffer.hpp"

#include "Graphics/OpenGL46/OpenGL.hpp"

namespace Vortex
{
    GL46IndexBuffer::GL46IndexBuffer(uint32 *indices, uint32 size)
    {
        glCreateBuffers(1, &id);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, id);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, size, indices, GL_STATIC_DRAW);
    }
    
    GL46IndexBuffer::~GL46IndexBuffer() noexcept
    {
        glDeleteBuffers(1, &id);
    }
    
    void GL46IndexBuffer::Bind() const
    {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, id);
    }
    
    void GL46IndexBuffer::Unbind() const
    {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    }
}