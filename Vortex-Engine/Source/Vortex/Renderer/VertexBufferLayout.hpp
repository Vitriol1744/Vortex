/*
 * Created by v1tr10l7 on 05.08.2024.
 * Copyright (c) 2024-2024, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include <Vortex/Core/Assertions.hpp>
#include <Vortex/Core/Types.hpp>

namespace Vortex
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
        eMat4,
    };

    inline usize ShaderDataTypeSize(ShaderDataType type)
    {
        switch (type)
        {
            case ShaderDataType::eBool: return sizeof(bool);
            case ShaderDataType::eInt: return 1 * sizeof(int);
            case ShaderDataType::eInt2: return 2 * sizeof(int);
            case ShaderDataType::eInt3: return 3 * sizeof(int);
            case ShaderDataType::eInt4: return 4 * sizeof(int);
            case ShaderDataType::eFloat: return 1 * sizeof(float);
            case ShaderDataType::eFloat2: return 2 * sizeof(float);
            case ShaderDataType::eFloat3: return 3 * sizeof(float);
            case ShaderDataType::eFloat4: return 4 * sizeof(float);
            case ShaderDataType::eMat3: return 3 * 3 * sizeof(float);
            case ShaderDataType::eMat4: return 4 * 4 * sizeof(float);
        }

        VtCoreAssertMsg(false, "Unknown ShaderDataType");
        std::unreachable();
    }
    inline usize ComponentCount(ShaderDataType type)
    {
        switch (type)
        {
            case ShaderDataType::eBool:
            case ShaderDataType::eInt:
            case ShaderDataType::eFloat: return 1;
            case ShaderDataType::eInt2:
            case ShaderDataType::eFloat2: return 2;
            case ShaderDataType::eInt3:
            case ShaderDataType::eFloat3: return 3;
            case ShaderDataType::eInt4:
            case ShaderDataType::eFloat4: return 4;
            case ShaderDataType::eMat3: return 3 * 3;
            case ShaderDataType::eMat4: return 4 * 4;
        }

        VtCoreAssertMsg(false, "Unknown ShaderDataType");
        std::unreachable();
    }

    struct VertexBufferElement
    {
        ShaderDataType Type{};
        u32            Offset = 0;
        u32            Size   = 0;

        VertexBufferElement() = default;
        VertexBufferElement(ShaderDataType type)
            : Type(type)
            , Size(ShaderDataTypeSize(type))
        {
        }
    };

    class VertexBufferLayout final
    {
      public:
        VertexBufferLayout() = default;
        VertexBufferLayout(InitializerList<VertexBufferElement>& elements)
            : m_Elements(elements)
        {

            u32 offset = 0;
            for (auto& element : m_Elements)
            {
                element.Offset = offset;
                offset += element.Size;
                m_Stride += element.Size;
            }
        }

        inline Vector<VertexBufferElement>::Iterator begin()
        {
            return m_Elements.begin();
        }
        inline Vector<VertexBufferElement>::Iterator end()
        {
            return m_Elements.end();
        }
        inline Vector<VertexBufferElement>::ConstIterator begin() const
        {
            return m_Elements.begin();
        }
        inline Vector<VertexBufferElement>::ConstIterator end() const
        {
            return m_Elements.end();
        }

        inline const auto& Elements() const { return m_Elements; }
        inline usize       ElementCount() const { return m_Elements.Size(); }
        inline u32         Stride() const { return m_Stride; }

      private:
        Vector<VertexBufferElement> m_Elements{};
        u32                         m_Stride = 0;
    };
}; // namespace Vortex
