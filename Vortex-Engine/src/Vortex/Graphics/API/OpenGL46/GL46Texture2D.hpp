//
// Created by vitriol1744 on 7/26/21.
//
#pragma once

#include "Core/Core.hpp"

#include "Graphics/API/ITexture2D.hpp"
#include "Graphics/API/OpenGL46/OpenGL.hpp"

namespace Vortex::Graphics
{
    class VT_API GL46Texture2D final : public ITexture2D
    {
        public:
            GL46Texture2D(strview filepath);

            virtual GLvoid Bind(uint8 slot) const noexcept override;
            virtual GLvoid Unbind()         const noexcept override;

        private:
            GLuint id = 0;
    };
}