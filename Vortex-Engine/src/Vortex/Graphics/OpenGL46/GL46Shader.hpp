//
// Created by vitriol1744 on 14.07.2021.
//
#pragma once

#include "Core/Core.hpp"
#include "Core/Math/Math.hpp"

#include "Graphics/IShader.hpp"
#include "Graphics/OpenGL46/OpenGL.hpp"

namespace Vortex::Graphics
{
    class VT_API GL46Shader final : public IShader
    {
        public:
            GL46Shader() noexcept = default;
            GL46Shader(std::string_view vertexPath, std::string_view fragmentPath, bool precompiled) noexcept;
            ~GL46Shader() noexcept override;

            GLvoid Bind() const noexcept override;
            GLvoid Unbind() const noexcept override;

            GLvoid Reload(std::string_view vertexPath, std::string_view fragmentPath, bool precompiled) noexcept override;
            GLvoid Load(std::string_view vertexPath, std::string_view fragmentPath, bool precompiled) noexcept override;

            GLvoid SetUniform1f(std::string_view name, float32 value) const noexcept override;
            GLvoid SetUniform2f(std::string_view name, Math::Vec2 vec) const noexcept override;
            GLvoid SetUniform3f(std::string_view name, Math::Vec3 vec) const noexcept override;
            GLvoid SetUniform4f(std::string_view name, Math::Vec4 vec) const noexcept override;
            GLvoid SetMat4f(std::string_view name, Math::Mat4 mat) const noexcept override;

        private:
            GLuint id = 0;

            static GLuint CompileShader(std::string_view source, GLenum shaderType);
    };
}