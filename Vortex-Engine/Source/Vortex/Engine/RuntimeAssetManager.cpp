/*
 * Created by v1tr10l7 on 11.07.2026.
 * Copyright (c) 2024-2026, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#include <Vortex/Core/Core.hpp>
#include <Vortex/Engine/RuntimeAssetManager.hpp>
#include <Vortex/Renderer/Mesh.hpp>

#include <filesystem>
#include <string>

namespace Vortex
{
    ::Ref<class Asset> RuntimeAssetManager::Asset(const AssetHandle& handle)
    {
        return nullptr;
    }
    ::Ref<TextureAsset> RuntimeAssetManager::LoadTexture2D(PathView path)
    {
        return CreateRef<TextureAsset>(path);
    }

    namespace
    {
        bool IsReadable(PathView path)
        {
            std::error_code error;
            return std::filesystem::is_regular_file(
                std::string(path.Raw(), path.Size()), error);
        }
    } // namespace

    AsyncAsset<Mesh> RuntimeAssetManager::LoadMeshAsync(PathView path)
    {
        return m_Loader.Load<Mesh>(path,
                                   [](PathView file) -> ::Ref<Mesh>
                                   {
                                       if (!IsReadable(file))
                                       {
                                           VtCoreErrorTag("Assets", "No mesh file '{}'", file);
                                           return nullptr;
                                       }
                                       return CreateRef<Mesh>(file);
                                   });
    }

    AsyncAsset<TextureAsset> RuntimeAssetManager::LoadTexture2DAsync(PathView path)
    {
        return m_Loader.Load<TextureAsset>(
            path,
            [](PathView file) -> ::Ref<TextureAsset>
            {
                if (!IsReadable(file))
                {
                    VtCoreErrorTag("Assets", "No texture file '{}'", file);
                    return nullptr;
                }
                return CreateRef<TextureAsset>(file);
            });
    }
}; // namespace Vortex
