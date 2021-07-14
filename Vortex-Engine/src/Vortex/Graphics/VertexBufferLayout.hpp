//
// Created by vitriol1744 on 11.07.2021.
//
#pragma once

#include "Core/Core.hpp"

#include <vector>

#undef Bool

namespace Vortex::Graphics
{
    enum class ShaderDataType
    {
        Bool,
        Int,
        Int2,
        Int3,
        Int4,
        Float,
        Float2,
        Float3,
        Float4,
        Mat3,
        Mat4
    };

    struct VertexBufferElement
    {
        VertexBufferElement(ShaderDataType type, uint32 offset)
                : type(type), offset(offset){ }

        ShaderDataType type;
        uint32         offset;
    };

    inline int32 GetSize(ShaderDataType type)
    {
        switch (type)
        {
            case ShaderDataType::Bool:      return sizeof(bool);
            case ShaderDataType::Int:       return sizeof(int);
            case ShaderDataType::Int2:      return sizeof(int) * 2;
            case ShaderDataType::Int3:      return sizeof(int) * 3;
            case ShaderDataType::Int4:      return sizeof(int) * 4;
            case ShaderDataType::Float:     return sizeof(float);
            case ShaderDataType::Float2:    return sizeof(float) * 2;
            case ShaderDataType::Float3:    return sizeof(float) * 3;
            case ShaderDataType::Float4:    return sizeof(float) * 4;
            case ShaderDataType::Mat3:      return sizeof(float) * 3 * 3;
            case ShaderDataType::Mat4:      return sizeof(float) * 4 * 4;
        }

        return 0;
    }

    inline int32 GetComponentCount(ShaderDataType type)
    {
        switch (type)
        {
            case ShaderDataType::Bool:
            case ShaderDataType::Int:
            case ShaderDataType::Float:     return 1;
            case ShaderDataType::Int2:
            case ShaderDataType::Float2:    return 2;
            case ShaderDataType::Int3:
            case ShaderDataType::Float3:    return 3;
            case ShaderDataType::Int4:
            case ShaderDataType::Float4:    return 4;
            case ShaderDataType::Mat3:      return 3 * 3;
            case ShaderDataType::Mat4:      return 4 * 4;
        }

        return 0;
    }

    class VertexBufferLayout
    {
        public:
            inline void AddElement(ShaderDataType type)
            {
                int32 size = GetSize(type);
                stride += size;
                elements.emplace_back(type, offset);
                offset += size;
            }

            VT_NODISCARD inline int32 GetStride() const { return stride; }

            std::vector<VertexBufferElement>::iterator begin() noexcept { return elements.begin(); }
            std::vector<VertexBufferElement>::iterator end() noexcept { return elements.end(); }

        private:
            std::vector<VertexBufferElement> elements;
            int32 stride = 0;
            uint32 offset = 0;
    };
}