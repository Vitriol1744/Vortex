//
// Created by vitriol1744 on 14.07.2021.
//
#include "GL46Shader.hpp"

#include <fstream>
#include <vector>

namespace Vortex::Graphics
{
    GL46Shader::GL46Shader(std::string_view vertexPath, std::string_view fragmentPath, bool precompiled) noexcept
    {
        Load(vertexPath, fragmentPath, precompiled);
    }

    GL46Shader::~GL46Shader() noexcept
    {
        glDeleteProgram(id);
    }

    GLvoid GL46Shader::Bind() const noexcept
    {
        glUseProgram(id);
    }

    GLvoid GL46Shader::Unbind() const noexcept
    {
        glUseProgram(0);
    }

    GLvoid GL46Shader::Reload(std::string_view vertexPath, std::string_view fragmentPath, bool precompiled) noexcept
    {
        glDeleteProgram(id);
        Load(vertexPath, fragmentPath, precompiled);
    }

    GLvoid GL46Shader::Load(std::string_view vertexPath, std::string_view fragmentPath, bool precompiled) noexcept
    {
        if (id) VT_CORE_LOG_WARN("If Shader was already loaded Reload should be invoked instead! Otherwise VRAM leaks may occur!");
        id = glCreateProgram();
        GLuint vertexShader     = 0;
        GLuint fragmentShader   = 0;

        std::ifstream ifs(vertexPath.data(), std::ios_base::binary);
        if (!ifs.is_open())
        {
            VT_CORE_ASSERT_MSG(false, "Invalid Vertex Shader Path!");
        }
        std::string vertexSource((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());

        ifs.close();
        ifs.open(fragmentPath.data(), std::ios_base::binary);
        if (!ifs.is_open())
        {
            VT_CORE_ASSERT_MSG(false, "Invalid Vertex Shader Path!");
        }
        std::string fragmentSource((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());

        if (precompiled)
        {
            vertexShader    = glCreateShader(GL_VERTEX_SHADER);
            fragmentShader  = glCreateShader(GL_FRAGMENT_SHADER);

            //TODO: Load Shaders Binary!
        }
        else
        {
            vertexShader    = CompileShader(vertexSource, GL_VERTEX_SHADER);
            fragmentShader  = CompileShader(fragmentSource, GL_FRAGMENT_SHADER);
        }

        glAttachShader(id, vertexShader);
        glAttachShader(id, fragmentShader);
        glLinkProgram(id);

        glValidateProgram(id);

        GLint status = 0;
        glGetProgramiv(id, GL_LINK_STATUS, &status);
        if (!status)
        {
            GLint maxLength = 0;
            glGetProgramiv(id, GL_INFO_LOG_LENGTH, &maxLength);

            std::vector<GLchar> infoLog(maxLength);

            glGetProgramInfoLog(id, maxLength, &maxLength, &infoLog[0]);

            VT_CORE_LOG_ERROR("\nFailed to Link Shader! Error:\n{}", infoLog.data());

            glDeleteProgram(id);
            glDeleteShader(vertexShader);
            glDeleteShader(fragmentShader);
            return;
        }

        glDetachShader(id, vertexShader);
        glDetachShader(id, fragmentShader);
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);
        glUseProgram(id);
    }

    GLvoid GL46Shader::SetUniform1f(std::string_view name, GLfloat value) const noexcept
    {
        GLint location = glGetUniformLocation(id, name.data());
        glUniform1f(location, value);
    }

    GLvoid GL46Shader::SetUniform2f(std::string_view name, Math::Vec2 vec) const noexcept
    {
        GLint location = glGetUniformLocation(id, name.data());
        glUniform2f(location, vec.x, vec.y);
    }

    GLvoid GL46Shader::SetUniform3f(std::string_view name, Math::Vec3 vec) const noexcept
    {
        GLint location = glGetUniformLocation(id, name.data());
        glUniform3f(location, vec.x, vec.y, vec.z);
    }

    GLvoid GL46Shader::SetUniform4f(std::string_view name, Math::Vec4 vec) const noexcept
    {
        GLint location = glGetUniformLocation(id, name.data());
        glUniform4f(location, vec.x, vec.y, vec.z, vec.w);
    }

    GLvoid GL46Shader::SetMat4f(std::string_view name, Math::Mat4 mat) const noexcept
    {
        GLint location = glGetUniformLocation(id, name.data());
        glUniformMatrix4fv(location, 1, GL_TRUE, mat.data);
    }

    GLuint GL46Shader::CompileShader(std::string_view source, GLenum shaderType)
    {
        GLuint shader = glCreateShader(shaderType);

        const GLchar* src = source.data();
        VT_CORE_LOG_INFO("\n{} Shader Source: {}", shaderType == GL_VERTEX_SHADER ? "Vertex" : "Fragment", src);
        glShaderSource(shader, 1, &src, nullptr);
        glCompileShader(shader);

        GLint status;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
        if (!status)
        {
            GLint maxLength = 0;
            glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &maxLength);

            std::vector<GLchar> infoLog(maxLength);

            glGetShaderInfoLog(shader, maxLength, nullptr, &infoLog[0]);

            VT_CORE_LOG_ERROR("\nFailed to Compile {} Shader! Error:\n{}", shaderType == GL_VERTEX_SHADER ? "Vertex" : "Fragment", infoLog.data());

            glDeleteShader(shader);
            return 0;
        }

        return shader;
    }
}
