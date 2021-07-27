//
// Created by vitriol1744 on 7/26/21.
//
#include "GL46Texture.hpp"

namespace Vortex::Graphics
{
    GL46Texture::GL46Texture()
    {

    }

    GLvoid GL46Texture::Bind() const noexcept
    {
        glBindTexture(GL_TEXTURE_2D, id);
    }

    GLvoid GL46Texture::Unbind() const noexcept
    {
        glBindTexture(GL_TEXTURE_2D, 0);
    }
}
