/*
 * Created by v1tr10l7 on 02.08.2024.
 * Copyright (c) 2024-2024, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#include <Vortex/Utility/ImageLoader.hpp>

#include <fstream>

namespace Vortex::ImageLoader
{
#pragma pack(push, 1)
    struct BmpHeader
    {
        u16 Signature;
        u32 Size;
        u32 Reserved;
        u32 PixelDataOffset;
    };

    struct DibHeader
    {
        u32 HeaderSize;
        u32 Width;
        u32 Height;
        u16 ColorPlaneCount;
        u16 BitsPerPixel;
        u32 CompressionMethod;
        u32 ImageSize;
        u32 HorizontalResolution;
        u32 VerticalResolution;
        u32 ColorCount;
        u32 ImportantColors;
    };
#pragma pack(pop)

    Expected<Scope<Pixel>, String> LoadBMP(Path path, i32& width, i32& height)
    {
        ::std::ifstream ifs(path.Raw(), ::std::ios::binary);
        if (!ifs.is_open())
            return Unexpected(
                fmt::format("Failed to open file, path: '{}'", path.Raw())
                    .data());

        BmpHeader bmpHeader;
        ifs.read(reinterpret_cast<char*>(&bmpHeader), sizeof(BmpHeader));
        if (bmpHeader.Signature != 0x4d42)
            return Unexpected(
                fmt::format("Invalid BMP signature, path: '{}'", path.Raw())
                    .data());

        DibHeader dibHeader;
        ifs.read(reinterpret_cast<char*>(&dibHeader), sizeof(DibHeader));

        u32 dibSize = dibHeader.HeaderSize;
        if (dibSize != 12 && dibSize != 40 && dibSize != 56 && dibSize != 108
            && dibSize != 124)
            return Unexpected(
                fmt::format("Invalid DIB header size, path: '{}'", path.Raw())
                    .data());

        Scope<Pixel[]> pixels = CreateScope<Pixel[]>(dibHeader.ImageSize);
        ifs.seekg(bmpHeader.PixelDataOffset);
        ifs.read(reinterpret_cast<char*>(pixels.Raw()), dibHeader.ImageSize);

        for (i64 i = 0; (i + 2) < dibHeader.ImageSize; i += 3)
            Swap(pixels[i], pixels[i + 2]);

        width  = dibHeader.Width;
        height = dibHeader.Height;
        return pixels;
    }
}; // namespace Vortex::ImageLoader
