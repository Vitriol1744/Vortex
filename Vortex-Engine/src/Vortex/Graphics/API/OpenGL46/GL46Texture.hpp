//
// Created by vitriol1744 on 7/26/21.
//
#pragma once

#include "Core/Core.hpp"

#include "Graphics/API/ITexture.hpp"
#include "Graphics/API/OpenGL46/OpenGL.hpp"

namespace Vortex::Graphics
{
    class VT_API GL46Texture final : public ITexture
    {
        public:
            GL46Texture();

            virtual GLvoid Bind() const noexcept override;
            virtual GLvoid Unbind() const noexcept override;

        private:
            GLuint id = 0;
    };
}