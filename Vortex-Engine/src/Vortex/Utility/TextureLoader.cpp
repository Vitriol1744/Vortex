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

    Scope<Pixel[]> TextureLoader::LoadTexture(std::string_view path, uint32& width, uint32& height, uint8& channels)
    {
        return LoadBMP(path, width, height, channels);
    }

    Scope<Pixel[]> TextureLoader::LoadBMP(std::string_view path, uint32& width, uint32& height, uint8& channels)
    {
        std::ifstream ifs(path.data(), std::ios::binary);
        if (!ifs) VTCoreLogWarn("Failed to Open BMP file at \"{}\"", path);
        
        Scope<Pixel[]> pixels;
        
        BMPHeader bmpHeader;
        DIBHeader dibHeader;
        
        ifs.read((char*)&bmpHeader, sizeof(bmpHeader));
        ifs.read((char*)&dibHeader, sizeof(dibHeader));
        if (bmpHeader.signature != 0x4D42)
        {
            VTCoreLogWarn("File at path: {} is not BMP file!", path.data());
            return nullptr;
        };
        
        pixels = CreateScope<Pixel[]>(dibHeader.imageSize);
        ifs.seekg(bmpHeader.offset);
        ifs.read((char*)pixels.get(), dibHeader.imageSize);
    
        // Channels have to be swapped because bitmap has BGR format.
        uint8 temp = 0;
        for (int64 i = 0; i < dibHeader.imageSize; i += 3)
        {
            temp          = pixels[i];
            pixels[i]     = pixels[i + 2];
            pixels[i + 2] = temp;
        }

        width = dibHeader.width;
        height = dibHeader.height;
        channels = 3;
        return pixels;
    }
    Scope<Pixel[]> TextureLoader::LoadPNG(std::string_view path, uint32& width, uint32& height, uint8& channels)
    {
        VT_CORE_ASSERT_MSG(false, "BMP Files are not supported!");
        return nullptr;
    }
}
