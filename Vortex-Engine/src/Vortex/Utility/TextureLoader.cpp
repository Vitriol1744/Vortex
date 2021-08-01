//
// Created by vitriol1744 on 20.07.2021.
//
#include "TextureLoader.hpp"

#include <fstream>

namespace Vortex::Utility
{
    #pragma pack(push, 1)
    struct BMPHeader
    {
        int16 signature;
        int32 size;
        int16 reserved1;
        int16 reserved2;
        int32 offset;
    };
    #pragma pack(pop)

    struct DIBHeader
    {
        int32 headersSize;
        int32 width;
        int32 height;
        int16 colorPlanesCount;
        int16 bitsPerPixel;
        int32 compressionMethod;
        int32 imageSize;
        int32 horizontalResolution;
        int32 verticalResolution;
        int32 colorsCount;
        int32 importantColors;
    };

    Scope<Pixel[]> TextureLoader::LoadTexture(std::string_view path)
    {
        return LoadPNG(path);
    }

    Scope<Pixel[]> TextureLoader::LoadBMP(std::string_view path)
    {
        std::ifstream ifs(path.data(), std::ios::binary);
        if (!ifs) VT_CORE_LOG_WARN("Failed to Open BMP file at \"{}\"", path);
        
        Scope<Pixel[]> pixels;
        
        BMPHeader bmpHeader;
        DIBHeader dibHeader;
        
        ifs.read((char*)&bmpHeader, sizeof(bmpHeader));
        ifs.read((char*)&dibHeader, sizeof(dibHeader));
        if (bmpHeader.signature != 0x4D42)
        {
            VT_CORE_LOG_WARN("File at path: {} is not BMP file!", path.data());
            return nullptr;
        };
        
        pixels = CreateScope<Pixel[]>(dibHeader.imageSize);
        ifs.seekg(bmpHeader.offset, std::ios_base::beg);
    
        // Channels have to be swapped because bitmap has BGR format.
        uint8 temp = 0;
        for (int64 i = 0; i < dibHeader.imageSize; i += 3)
        {
            temp          = pixels[i];
            pixels[i]     = pixels[i + 2];
            pixels[i + 2] = temp;
        }

        return pixels;
    }

    Scope<Pixel[]> TextureLoader::LoadPNG(std::string_view path)
    {
        VT_CORE_ASSERT_MSG(false, "BMP Files are not supported!");
        return nullptr;
    }
}
