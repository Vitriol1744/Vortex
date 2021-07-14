//
// Created by Vitriol1744 on 26.06.2021.
//
#pragma once

#include "Core/Core.hpp"

#include "Graphics/IRendererAPI.hpp"
#include "Graphics/OpenGL46/OpenGL.hpp"

namespace Vortex::Graphics
{
    class GL46RendererAPI final : public IRendererAPI
    {
        public:
            GLvoid SetClearColor(Math::Vec4 color) noexcept override;
            GLvoid Clear() noexcept override;

            GLvoid DrawIndexed(const Ref<IMesh>& mesh, uint32_t indicesCount) override;
    };
}


