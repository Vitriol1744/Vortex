/*
 * Created by v1tr10l7 on 02.08.2024.
 * Copyright (c) 2024-2024, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#include "vtpch.hpp"

#include "Vortex/Utility/ImageLoader.hpp"

namespace Vortex::ImageLoader
{
#pragma pack(push, 1)
    struct BmpHeader
    {
        i16 Signature;
        i32 Size;
        i32 Reserved;
        i32 Offset;
    };

    struct DibHeader
    {
        i32 HeaderSize;
        i32 Width;
        i32 Height;
        i16 ColorPlaneCount;
        i16 BitsPerPixel;
        i32 CompressionMethod;
        i32 ImageSize;
        i32 HorizontalResolution;
        i32 VerticalResolution;
        i32 ColorCount;
        i32 ImportantColors;
    };
#pragma pack(pop)

    std::expected<Scope<Pixel[]>, std::string>
    LoadBMP(std::filesystem::path path, i32& width, i32& height)
    {
        std::ifstream ifs(path, std::ios::binary);
        if (!ifs.is_open())
            return std::unexpected(
                fmt::format("Failed to open file, path: '{}'", path.string()));

        BmpHeader bmpHeader;
        ifs.read(reinterpret_cast<char*>(&bmpHeader), sizeof(BmpHeader));
        if (bmpHeader.Signature != 0x4d42)
            return std::unexpected(fmt::format(
                "Invalid BMP signature, path: '{}'", path.string()));

        DibHeader dibHeader;
        ifs.read(reinterpret_cast<char*>(&dibHeader), sizeof(DibHeader));

        Scope<Pixel[]> pixels = CreateScope<Pixel[]>(dibHeader.ImageSize);
        ifs.seekg(bmpHeader.Offset);
        ifs.read(reinterpret_cast<char*>(pixels.get()), dibHeader.ImageSize);

        // The bitmap is in the BGR format, we have to swap bytes to make it RGB
        u8 temp = 0;
        for (i64 i = 0; i < dibHeader.ImageSize; i += 3)
        {
            temp          = pixels[i];
            pixels[i]     = pixels[i + 2];
            pixels[i + 2] = temp;
        }

        width  = dibHeader.Width;
        height = dibHeader.Height;
        return pixels;
    }
}; // namespace Vortex::ImageLoader
