//
// Created by vitriol1744 on 20.07.2021.
//
#include "TextureLoader.hpp"

namespace Vortex::Utility
{
    Ref<Pixel> TextureLoader::LoadTexture(std::string_view path)
    {
        return LoadPNG(path);
    }

    Ref<Pixel> TextureLoader::LoadBMP(std::string_view path)
    {
        VT_CORE_ASSERT_MSG(false, "BMP Files are not supported!");
        return nullptr;
    }

    Ref<Pixel> TextureLoader::LoadPNG(std::string_view path)
    {
        VT_CORE_ASSERT_MSG(false, "BMP Files are not supported!");
        return nullptr;
    }
}
