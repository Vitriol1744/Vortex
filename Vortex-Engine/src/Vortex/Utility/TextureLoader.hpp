//
// Created by vitriol1744 on 20.07.2021.
//
#pragma once

#include "Vortex/Core/Core.hpp"

#include <string>

namespace Vortex::Utility
{
    using Pixel = uint8;

    class VT_API TextureLoader
    {
        public:
            Ref<Pixel[]> LoadTexture(std::string_view path);

        private:
            Ref<Pixel[]> LoadBMP(std::string_view path);
            Ref<Pixel[]> LoadPNG(std::string_view path);
    };
}

