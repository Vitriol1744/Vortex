//
// Created by vitriol1744 on 11.07.2021.
//
#pragma once

#include "Vortex/Core/Core.hpp"

#include <vector>

#undef Bool

namespace Vortex::Graphics
{
    enum class ShaderDataType
    {
        eBool,
        eInt,
        eInt2,
        eInt3,
        eInt4,
        eFloat,
        eFloat2,
        eFloat3,
        eFloat4,
        eMat3,
        eMat4
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
            case ShaderDataType::eBool:      return sizeof(bool);
            case ShaderDataType::eInt:       return sizeof(int);
            case ShaderDataType::eInt2:      return sizeof(int) * 2;
            case ShaderDataType::eInt3:      return sizeof(int) * 3;
            case ShaderDataType::eInt4:      return sizeof(int) * 4;
            case ShaderDataType::eFloat:     return sizeof(float);
            case ShaderDataType::eFloat2:    return sizeof(float) * 2;
            case ShaderDataType::eFloat3:    return sizeof(float) * 3;
            case ShaderDataType::eFloat4:    return sizeof(float) * 4;
            case ShaderDataType::eMat3:      return sizeof(float) * 3 * 3;
            case ShaderDataType::eMat4:      return sizeof(float) * 4 * 4;
        }

        return 0;
    }

    inline int32 GetComponentCount(ShaderDataType type)
    {
        switch (type)
        {
            case ShaderDataType::eBool:
            case ShaderDataType::eInt:
            case ShaderDataType::eFloat:     return 1;
            case ShaderDataType::eInt2:
            case ShaderDataType::eFloat2:    return 2;
            case ShaderDataType::eInt3:
            case ShaderDataType::eFloat3:    return 3;
            case ShaderDataType::eInt4:
            case ShaderDataType::eFloat4:    return 4;
            case ShaderDataType::eMat3:      return 3 * 3;
            case ShaderDataType::eMat4:      return 4 * 4;
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