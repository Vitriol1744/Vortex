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
                case ShaderDataType::eBool:      return GL_BOOL;
                case ShaderDataType::eInt:
                case ShaderDataType::eInt2:
                case ShaderDataType::eInt3:
                case ShaderDataType::eInt4:      return GL_INT;
                case ShaderDataType::eFloat:
                case ShaderDataType::eFloat2:
                case ShaderDataType::eFloat3:
                case ShaderDataType::eFloat4:
                case ShaderDataType::eMat3:
                case ShaderDataType::eMat4:      return GL_FLOAT;

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

    GL46VertexArray::~GL46VertexArray() noexcept
    {
        glDeleteVertexArrays(1, &id);
    }

    GLvoid GL46VertexArray::Bind()
    {
        glBindVertexArray(id);
    }

    GLvoid GL46VertexArray::Unbind() noexcept
    {
        glBindVertexArray(0);
    }

    GLvoid GL46VertexArray::AddVertexBuffer(const Ref<IVertexBuffer>& vertexBuffer)
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
                case ShaderDataType::eBool:
                case ShaderDataType::eInt:
                case ShaderDataType::eInt2:
                case ShaderDataType::eInt3:
                case ShaderDataType::eInt4:
                    glVertexAttribIPointer(index, GetComponentCount(e.type), GLType(e.type), layout.GetStride(), reinterpret_cast<const GLvoid*>(e.offset));
                    glEnableVertexAttribArray(index);
                    index++;
                    break;
                case ShaderDataType::eFloat:
                case ShaderDataType::eFloat2:
                case ShaderDataType::eFloat3:
                case ShaderDataType::eFloat4:
                    glVertexAttribPointer(index, GetComponentCount(e.type), GLType(e.type), GL_FALSE, layout.GetStride(), reinterpret_cast<const GLvoid*>(e.offset));
                    glEnableVertexAttribArray(index);
                    index++;
                    break;
                case ShaderDataType::eMat3:
                case ShaderDataType::eMat4:
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

    GLvoid GL46VertexArray::SetIndexBuffer(const Ref<IIndexBuffer>& indexBuffer)
    {
        Bind();
        indexBuffer->Bind();
        this->indexBuffer = indexBuffer;
    }
}
