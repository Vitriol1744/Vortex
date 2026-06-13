/*
 * Created by v1tr10l7 on 24.05.2026.
 * Copyright (c) 2024-2026, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include <Vortex/Asset/AssetManager.hpp>
#include <Vortex/Renderer/API/Material.hpp>
#include <Vortex/Renderer/API/Texture2D.hpp>
#include <Vortex/Renderer/API/UniformBuffer.hpp>
#include <Vortex/Renderer/Camera.hpp>
#include <Vortex/Renderer/Mesh.hpp>

namespace Vortex
{
    struct TagComponent
    {
        String Name    = "";

        TagComponent() = default;
        TagComponent(String name)
            : Name(name)
        {
        }
        TagComponent(const char* name)
            : Name(name)
        {
        }

        inline operator const String&() const { return Name; }
        inline operator const char*() const { return Name.Raw(); }
    };
    struct TransformComponent
    {
        Math::Vec3 Translation = {0.0f, 0.0f, 0.0f};
        Math::Vec3 Scale       = {1.0f, 1.0f, 1.0f};
        Quat       Rotation{};

        inline     operator Mat4() const
        {
            return glm::translate(Mat4(1.0f), Translation)
                 * glm::mat4_cast(Rotation) * glm::scale(Mat4(1.0f), Scale);
        }
    };
    struct CameraComponent
    {
        class Camera Camera;
        bool         Primary          = true;
        bool         FixedAspectRatio = false;
    };
    struct SpriteComponent
    {
        Math::Vec4 Color;
    };
    struct MeshComponent
    {
        Ref<class Mesh>          Mesh          = nullptr;
        Ref<class Material>      Material      = nullptr;
        Ref<class Texture2D>     Texture       = nullptr;
        Ref<class UniformBuffer> UniformBuffer = nullptr;

        MeshComponent()
        {
            auto shader = AssetManager::ShaderLibrary().Get("TextureShader");
            Material    = Material::Create(shader);
        }
        MeshComponent(const Ref<class Mesh>     mesh,
                      const Ref<class Material> material,
                      const Ref<Texture2D>      texture)
            : Mesh(mesh)
            , Material(material)
            , Texture(texture)
        {
        }
    };
}; // namespace Vortex
