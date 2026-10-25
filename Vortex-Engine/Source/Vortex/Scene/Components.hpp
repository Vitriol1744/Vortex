/*
 * Created by v1tr10l7 on 24.05.2026.
 * Copyright (c) 2024-2026, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include <Vortex/Engine/Application.hpp>
#include <Vortex/Renderer/Mesh.hpp>
#include <Vortex/Renderer/RHI/Material.hpp>
#include <Vortex/Renderer/RHI/UniformBuffer.hpp>
#include <Vortex/Renderer/Renderer.hpp>
#include <Vortex/Renderer/TextureAsset.hpp>
#include <Vortex/Scene/Camera.hpp>
#include <Vortex/Serialization/Archive.hpp>

namespace Vortex
{
    inline void SerializeValue(ArchiveWriter& writer, StringView key,
                               const Quat& q)
    {
        writer.BeginObject(key);
        writer.Write("x", q.x);
        writer.Write("y", q.y);
        writer.Write("z", q.z);
        writer.Write("w", q.w);
        writer.EndObject();
    }
    inline void SerializeValue(ArchiveWriter& writer, StringView key,
                               const Math::Vec3& v)
    {
        writer.BeginObject(key);
        writer.Write("x", v.x);
        writer.Write("y", v.y);
        writer.Write("z", v.z);
        writer.EndObject();
    }
    inline void SerializeValue(ArchiveWriter& writer, StringView key,
                               const Math::Vec4& v)
    {
        writer.BeginObject(key);
        writer.Write("x", v.x);
        writer.Write("y", v.y);
        writer.Write("z", v.z);
        writer.Write("w", v.w);
        writer.EndObject();
    }
    inline bool DeserializeValue(ArchiveReader& reader, StringView key,
                                 Math::Vec3& v)
    {
        if (!reader.BeginObject(key)) return false;

        reader.Read("x", v.x);
        reader.Read("y", v.y);
        reader.Read("z", v.z);

        reader.EndObject();
        return true;
    }

    inline bool DeserializeValue(ArchiveReader& reader, StringView key,
                                 Math::Vec4& v)
    {
        if (!reader.BeginObject(key)) return false;

        reader.Read("x", v.x);
        reader.Read("y", v.y);
        reader.Read("z", v.z);
        reader.Read("w", v.w);

        reader.EndObject();
        return true;
    }

    inline bool DeserializeValue(ArchiveReader& reader, StringView key, Quat& q)
    {
        if (!reader.BeginObject(key)) return false;

        reader.Read("x", q.x);
        reader.Read("y", q.y);
        reader.Read("z", q.z);
        reader.Read("w", q.w);

        reader.EndObject();
        return true;
    }
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

        void   Serialize(ArchiveWriter& archive) const
        {
            archive.Write("tag", Name.View());
        }
        bool Deserialize(ArchiveReader& archive)
        {
            return archive.Read("tag", Name);
        }
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

        inline glm::vec3 RotationEuler() const
        {
            return glm::eulerAngles(Rotation);
        }
        inline void SetRotationEuler(const glm::vec3& euler)
        {
            Rotation = glm::quat(euler);
        }

        void Serialize(ArchiveWriter& archive) const
        {
            SerializeValue(archive, "translation", Translation);
            SerializeValue(archive, "scale", Scale);
            SerializeValue(archive, "rotation", Rotation);
        }
        bool Deserialize(ArchiveReader& archive)
        {
            DeserializeValue(archive, "translation", Translation);
            DeserializeValue(archive, "scale", Scale);
            DeserializeValue(archive, "rotation", Rotation);

            return true;
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

        void       Serialize(ArchiveWriter& archive) const
        {
            SerializeValue(archive, "color", Color);
        }
        bool Deserialize(ArchiveReader& archive)
        {
            return DeserializeValue(archive, "color", Color);
        }
    };
    struct MeshComponent
    {
        Ref<class Mesh>          Mesh          = nullptr;
        Ref<class Material>      Material      = nullptr;
        Ref<class TextureAsset>  Texture       = nullptr;
        Ref<class UniformBuffer> UniformBuffer = nullptr;

        MeshComponent()
        {
            auto shader = Renderer::ShaderLibrary().Get("TextureShader");
            Material    = Material::Create(shader);
        }
        MeshComponent(const Ref<class Mesh>     mesh,
                      const Ref<class Material> material,
                      const Ref<TextureAsset>   texture)
            : Mesh(mesh)
            , Material(material)
            , Texture(texture)
        {
        }

        void Serialize(ArchiveWriter& archive) const
        {
            archive.Write("meshPath", Mesh ? Mesh->FilePath() : ""_pv);
            archive.Write("texturePath", Texture ? Texture->FilePath() : ""_pv);
        }
        bool Deserialize(ArchiveReader& reader)
        {
            Path meshPath;
            Path texturePath;

            if (!reader.Read("meshPath", meshPath)) meshPath = "";
            if (!reader.Read("texturePath", texturePath)) texturePath = "";

            Mesh
                = !meshPath.Empty() ? CreateRef<class Mesh>(meshPath) : nullptr;
            Texture = !texturePath.Empty()
                        ? CreateRef<TextureAsset>(texturePath)
                        : nullptr;

            return true;
        }
    };
    struct ScriptComponent
    {
        StringView ClassName = "Gameplay.PlayerController";
    };

    inline bool DeserializeValue(ArchiveReader& reader, StringView key,
                                 TagComponent& component)
    {
        if (!reader.BeginObject(key)) return false;
        bool success = component.Deserialize(reader);
        reader.EndObject();
        return success;
    }

    inline bool DeserializeValue(ArchiveReader& reader, StringView key,
                                 TransformComponent& component)
    {
        if (!reader.BeginObject(key)) return false;
        bool success = component.Deserialize(reader);
        reader.EndObject();
        return success;
    }

    inline bool DeserializeValue(ArchiveReader& reader, StringView key,
                                 SpriteComponent& component)
    {
        if (!reader.BeginObject(key)) return false;
        bool success = component.Deserialize(reader);
        reader.EndObject();
        return success;
    }

    inline bool DeserializeValue(ArchiveReader& reader, StringView key,
                                 MeshComponent& component)
    {
        if (!reader.BeginObject(key)) return false;
        bool success = component.Deserialize(reader);
        reader.EndObject();
        return success;
    }
}; // namespace Vortex
