//
// Created by vitriol1744 on 11.07.2021.
//
#include "GL46VertexArray.hpp"
#include "OpenGL.hpp"

#undef Bool

namespace Vortex::Graphics
{
    namespace
    {
        GLenum GLType(ShaderDataType type) noexcept
        {
            switch (type)
            {
                case ShaderDataType::Bool:      return GL_BOOL;
                case ShaderDataType::Int:
                case ShaderDataType::Int2:
                case ShaderDataType::Int3:
                case ShaderDataType::Int4:      return GL_INT;
                case ShaderDataType::Float:
                case ShaderDataType::Float2:
                case ShaderDataType::Float3:
                case ShaderDataType::Float4:
                case ShaderDataType::Mat3:
                case ShaderDataType::Mat4:      return GL_FLOAT;

                default:
                    VT_CORE_ASSERT_MSG(false, "Unknown Shader Type!");
            }

            return 0;
        }
    }

    GL46VertexArray::GL46VertexArray() noexcept
    {
        glCreateVertexArrays(1, &id);
        glBindVertexArray(id);
    }

    GLvoid GL46VertexArray::Bind()
    {
        glBindVertexArray(id);
    }

    GLvoid GL46VertexArray::Unbind() noexcept
    {
        glBindVertexArray(0);
    }

    GLvoid GL46VertexArray::AddVertexBuffer(Ref<IVertexBuffer> vertexBuffer)
    {
        Bind();
        vertexBuffer->Bind();
        vertexBuffers.push_back(vertexBuffer);

        auto layout = vertexBuffer->GetLayout();
        GLint index = 0;
        for (auto& e : layout)
        {
            switch (e.type)
            {
                case ShaderDataType::Bool:
                case ShaderDataType::Int:
                case ShaderDataType::Int2:
                case ShaderDataType::Int3:
                case ShaderDataType::Int4:
                    glVertexAttribIPointer(index, GetComponentCount(e.type), GLType(e.type), layout.GetStride(), reinterpret_cast<const GLvoid*>(e.offset));
                    glEnableVertexAttribArray(index);
                    index++;
                    break;
                case ShaderDataType::Float:
                case ShaderDataType::Float2:
                case ShaderDataType::Float3:
                case ShaderDataType::Float4:
                    glVertexAttribPointer(index, GetComponentCount(e.type), GLType(e.type), GL_FALSE, layout.GetStride(), reinterpret_cast<const GLvoid*>(e.offset));
                    glEnableVertexAttribArray(index);
                    index++;
                    break;
                case ShaderDataType::Mat3:
                case ShaderDataType::Mat4:
                {
                    uint8_t count = GetComponentCount(e.type);
                    for (uint8_t i = 0; i < count; i++)
                    {
                        glEnableVertexAttribArray(index);
                        glVertexAttribPointer(index, count, GLType(e.type), GL_FALSE, layout.GetStride(), reinterpret_cast<const GLvoid*>(e.offset + sizeof(float) * count * i));
                        glVertexAttribDivisor(index, 1);
                        index++;
                    }
                    break;
                }
                default:
                    VT_CORE_ASSERT_MSG(false, "Unknown ShaderDataType!");
            }
        }
    }

    GLvoid GL46VertexArray::SetIndexBuffer(Ref<IIndexBuffer> indexBuffer)
    {
        Bind();
        indexBuffer->Bind();
        this->indexBuffer = indexBuffer;
    }
}
