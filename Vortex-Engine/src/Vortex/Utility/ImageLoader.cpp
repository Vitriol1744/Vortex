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

        u32 dibSize = dibHeader.HeaderSize;
        if (dibSize != 12 && dibSize != 40 && dibSize != 56 && dibSize != 108
            && dibSize != 124)
            return std::unexpected(fmt::format(
                "Invalid DIB header size, path: '{}'", path.string()));

        Scope<Pixel[]> pixels = CreateScope<Pixel[]>(dibHeader.ImageSize);
        ifs.seekg(bmpHeader.PixelDataOffset);
        ifs.read(reinterpret_cast<char*>(pixels.get()), dibHeader.ImageSize);

        for (i64 i = 0; (i + 2) < dibHeader.ImageSize; i += 3)
            std::swap(pixels[i], pixels[i + 2]);

        width  = dibHeader.Width;
        height = dibHeader.Height;
        return pixels;
    }
}; // namespace Vortex::ImageLoader
