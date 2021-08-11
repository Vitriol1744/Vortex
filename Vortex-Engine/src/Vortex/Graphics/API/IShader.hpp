//
// Created by vitriol1744 on 14.07.2021.
//
#pragma once

#include "Vortex/Core/Core.hpp"

namespace Vortex
{
    namespace Graphics
    {
        class VT_API IShader : public NonCopyable<IShader>
        {
            public:
                virtual ~IShader() noexcept = default;

                virtual void Bind() const noexcept = 0;
                virtual void Unbind() const noexcept = 0;

                virtual HashedString GetName() const noexcept = 0;

                virtual void Reload(strview vertexPath, strview pixelPath, bool precompiled) noexcept = 0;
                virtual void Load(strview vertexPath, strview pixelPath, bool precompiled) = 0;

                virtual void SetUniform1i(strview name, int32 value) const = 0;
                virtual void SetUniform1f(strview name, float32 value) const = 0;
                virtual void SetUniform2f(strview name, Math::Vec2 vec) const = 0;
                virtual void SetUniform3f(strview name, Math::Vec3 vec) const = 0;
                virtual void SetUniform4f(strview name, Math::Vec4 vec) const = 0;
                virtual void SetUniformMat4f(strview name, Math::Mat4 mat) const = 0;

                static Ref<IShader> Create(strview name, strview vertexPath, strview pixelPath, bool precompiled = true);
                static Ref<IShader> Create(strview vertexPath, strview pixelPath, bool precompiled = true);
        };
    }

    template class VT_API NonCopyable<Graphics::IShader>;
}