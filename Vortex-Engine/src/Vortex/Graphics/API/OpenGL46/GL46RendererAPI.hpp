//
// Created by Vitriol1744 on 26.06.2021.
//
#pragma once

#include "Vortex/Core/Core.hpp"

#include "Vortex/Graphics/API/IRendererAPI.hpp"
#include "Vortex/Platform/OpenGL.hpp"

namespace Vortex::Graphics
{
    class GL46RendererAPI final : public IRendererAPI
    {
        public:
            virtual GLvoid InitializeImpl() override;

            virtual GLvoid SetClearColorImpl(Math::Vec4 color) override;
            virtual GLvoid SetViewportImpl(const Math::Rectangle& rect) override;

            virtual GLvoid ClearImpl() noexcept override;
            virtual GLvoid DrawIndexedImpl(const Ref<IVertexArray>& mesh, uint32_t indicesCount) override;

            static GLboolean PostInitialize();

        private:
            static GLvoid APIENTRY ErrorCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const GLvoid* userParam);
    };
}


