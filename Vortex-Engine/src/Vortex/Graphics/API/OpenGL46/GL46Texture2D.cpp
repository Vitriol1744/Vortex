//
// Created by vitriol1744 on 7/26/21.
//
#include "GL46Texture2D.hpp"

#include "Vortex/Utility/TextureLoader.hpp"

namespace Vortex::Graphics
{
    using namespace Utility;

    GL46Texture2D::GL46Texture2D(strview filepath)
    {
        glGenTextures(1, &id);
        glBindTexture(GL_TEXTURE_2D, id);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        uint32 width, height;
        uint8 channels;
        Scope<Pixel[]> pixels = TextureLoader::LoadTexture(filepath, width, height, channels);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, pixels.get());
    }

    GLvoid GL46Texture2D::Bind(uint8 slot) const noexcept
    {
        glActiveTexture(GL_TEXTURE0 + slot);
        glBindTexture(GL_TEXTURE_2D, id);
    }

    GLvoid GL46Texture2D::Unbind() const noexcept
    {
        glBindTexture(GL_TEXTURE_2D, 0);
    }
}
