//
// Created by vitriol1744 on 14.07.2021.
//
#pragma once

#include "Vortex/Core/Core.hpp"
#include "Vortex/Core/Math/Math.hpp"

#include "Vortex/Graphics/API/IShader.hpp"
#include "Vortex/Platform/OpenGL.hpp"

#include <unordered_map>

namespace Vortex::Graphics
{
    class VT_API GL46Shader final : public IShader
    {
        public:
            GL46Shader() = default;
            GL46Shader(strview name, strview vertexPath, strview fragmentPath, bool precompiled);
            GL46Shader(std::string_view vertexPath, std::string_view fragmentPath, bool precompiled);
            ~GL46Shader() noexcept override;

            GLvoid Bind() const noexcept override;
            GLvoid Unbind() const noexcept override;

            GLvoid Reload(std::string_view vertexPath, std::string_view fragmentPath, bool precompiled) override;
            GLvoid Load(std::string_view vertexPath, std::string_view fragmentPath, bool precompiled) override;

            inline HashedString GetName() const noexcept override { return name; }

            GLvoid SetUniform1i(strview name, int32 value) const override;
            GLvoid SetUniform1f(strview name, float32 value) const override;
            GLvoid SetUniform2f(strview name, Math::Vec2 vec) const override;
            GLvoid SetUniform3f(strview name, Math::Vec3 vec) const override;
            GLvoid SetUniform4f(strview name, Math::Vec4 vec) const override;
            virtual GLvoid SetUniformMat4f(strview name, glm::mat4 mat) const override;

        private:
            GLuint id = 0;
            HashedString name;

            mutable std::unordered_map<uint64, GLuint> uniformCache;

            GLuint GetUniformLocation(const GLchar* uniform) const;
            static GLuint CompileShader(strview source, GLenum shaderType, strview shaderPath);
    };
}