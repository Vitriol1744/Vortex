//
// Created by Vitriol1744 on 26.06.2021.
//
#pragma once

#include "Core/Core.hpp"

#include "Graphics/API/IRendererAPI.hpp"
#include "OpenGL.hpp"

namespace Vortex::Graphics
{
    class GL46RendererAPI final : public IRendererAPI
    {
        public:
            GLvoid SetClearColorImpl(Math::Vec4 color) override;
            GLvoid SetViewportImpl(const Math::Rectangle& rect) override;

            GLvoid ClearImpl() noexcept override;
            GLvoid DrawIndexedImpl(const Ref<IVertexArray>& mesh, uint32_t indicesCount) override;

            static GLvoid Initialize();

        private:
            static GLvoid ErrorCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const GLvoid* userParam);
    };
}


