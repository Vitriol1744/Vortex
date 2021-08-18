//
// Created by vitriol1744 on 14.07.2021.
//
#include "GL46Shader.hpp"

#include "Vortex/Core/HashedString.hpp"

#include <fstream>
#include <vector>

namespace Vortex::Graphics
{
    GL46Shader::GL46Shader(strview name, strview vertexPath, strview fragmentPath, bool precompiled)
    {
        this->name = HashedString(name);

        Load(vertexPath, fragmentPath, precompiled);
    }

    GL46Shader::GL46Shader(strview vertexPath, strview fragmentPath, bool precompiled)
    {
        auto lastDot = vertexPath.find_last_of(".");
        name = HashedString(vertexPath.substr(lastDot + 1));

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

    GLvoid GL46Shader::Reload(strview vertexPath, strview fragmentPath, bool precompiled)
    {
        glDeleteProgram(id);
        Load(vertexPath, fragmentPath, precompiled);
    }

    GLvoid GL46Shader::Load(strview vertexPath, strview fragmentPath, bool precompiled)
    {
        if (id) VTCoreLogWarn("If Shader was already loaded Reload should be invoked instead! Otherwise VRAM leaks may occur!");
        id = glCreateProgram();
        GLuint vertexShader;
        GLuint fragmentShader;

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
            VTCoreLogInfo("VertexShader:\n {}\n\nPixelShader:\n {}", vertexSource, fragmentSource);
            vertexShader    = CompileShader(vertexSource, GL_VERTEX_SHADER, vertexPath);
            fragmentShader  = CompileShader(fragmentSource, GL_FRAGMENT_SHADER, fragmentPath);
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

            VTCoreLogError("\nFailed to Link Shader! Error:\n{}", infoLog.data());

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

    GLvoid GL46Shader::SetUniform1i(strview name, int32 value) const
    {
        glUniform1i(GetUniformLocation(name.data()), value);
    }
    GLvoid GL46Shader::SetUniform1f(strview name, GLfloat value) const
    {
        glUniform1f(GetUniformLocation(name.data()), value);
    }
    GLvoid GL46Shader::SetUniform2f(strview name, Math::Vec2 vec) const
    {
        glUniform2f(GetUniformLocation(name.data()), vec.x, vec.y);
    }
    GLvoid GL46Shader::SetUniform3f(strview name, Math::Vec3 vec) const
    {
        glUniform3f(GetUniformLocation(name.data()), vec.x, vec.y, vec.z);
    }
    GLvoid GL46Shader::SetUniform4f(strview name, Math::Vec4 vec) const
    {
        glUniform4f(GetUniformLocation(name.data()), vec.x, vec.y, vec.z, vec.w);
    }
    GLvoid GL46Shader::SetUniformMat4f(strview name, glm::mat4 mat) const
    {
        glUniformMatrix4fv(GetUniformLocation(name.data()), 1, GL_FALSE, glm::value_ptr(mat));
    }

    GLuint GL46Shader::GetUniformLocation(const GLchar* uniform) const
    {
        return glGetUniformLocation(id, uniform);
        HashedString uniformName(uniform);
        if (uniformCache.find(uniformName.stringID) != uniformCache.end()) return uniformCache[uniformName.stringID];

        GLint location = glGetUniformLocation(id, uniform);
        if (location < 0)
        {
            VTCoreLogWarn("Failed to find location of uniform with name: {}", uniform);
            return 0;
        }

        uniformCache[uniformName.stringID] = location;
        return location;
    }
    GLuint GL46Shader::CompileShader(strview source, GLenum shaderType, strview shaderPath)
    {
        GLuint shader = glCreateShader(shaderType);

        const GLchar* src = source.data();

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

            VTCoreLogError("\nFailed to Compile {} Shader!\nPath: {}\nError:\n{}",
             shaderType == GL_VERTEX_SHADER ? "Vertex" : "Fragment", shaderPath, infoLog.data());

            glDeleteShader(shader);
            return 0;
        }

        return shader;
    }
}
