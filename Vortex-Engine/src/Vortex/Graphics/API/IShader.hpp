//
// Created by vitriol1744 on 14.07.2021.
//
#pragma once

#include "Vortex/Core/Core.hpp"

#include <string_view>

namespace Vortex::Graphics
{
    class VT_API IShader : public NonCopyable<IShader>
    {
        public:
            virtual ~IShader() noexcept = default;

            virtual void Bind() const noexcept = 0;
            virtual void Unbind() const noexcept = 0;

            virtual void Reload(std::string_view vertexPath, std::string_view pixelPath, bool precompiled) noexcept = 0;
            virtual void Load(std::string_view vertexPath, std::string_view pixelPath, bool precompiled) = 0;

            virtual void SetUniform1f(std::string_view name, float32 value) const noexcept = 0;
            virtual void SetUniform2f(std::string_view name, Math::Vec2 vec) const noexcept = 0;
            virtual void SetUniform3f(std::string_view name, Math::Vec3 vec) const noexcept = 0;
            virtual void SetUniform4f(std::string_view name, Math::Vec4 vec) const noexcept = 0;
            virtual void SetUniformMat4f(std::string_view name, Math::Mat4 mat) const noexcept = 0;

            static Ref<IShader> Create(std::string_view vertexPath, std::string_view pixelPath, bool precompiled = true);
    };
}