/*
 * Created by v1tr10l7 on 24.05.2026.
 * Copyright (c) 2024-2026, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include <Vortex/Engine/Application.hpp>
#include <Vortex/Renderer/Mesh.hpp>
#include <Vortex/Renderer/ShaderLibrary.hpp>
#include <Vortex/Renderer/TextureAsset.hpp>
#include <Vortex/Scene/Camera.hpp>
#include <Vortex/Serialization/Archive.hpp>

#include <entt/entt.hpp>

#include <cstring>

import Vortex.Renderer.Material;
import Vortex.Renderer;

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

        /// translate * rotate * scale, written out: the same matrix as the
        /// glm product without two 4x4 multiplies (it runs for every drawn
        /// actor every frame, and unoptimised glm is slow).
        inline     operator Mat4() const
        {
            const f32 x = Rotation.x, y = Rotation.y, z = Rotation.z, w = Rotation.w;
            const f32 xx = x * x, yy = y * y, zz = z * z;
            const f32 xy = x * y, xz = x * z, yz = y * z;
            const f32 wx = w * x, wy = w * y, wz = w * z;
            const f32 sx = Scale.x, sy = Scale.y, sz = Scale.z;
            // Column-major: rotation columns scaled per axis, then translation.
            return Mat4(sx * (1.0f - 2.0f * (yy + zz)), sx * 2.0f * (xy + wz),
                        sx * 2.0f * (xz - wy), 0.0f,
                        sy * 2.0f * (xy - wz), sy * (1.0f - 2.0f * (xx + zz)),
                        sy * 2.0f * (yz + wx), 0.0f,
                        sz * 2.0f * (xz + wy), sz * 2.0f * (yz - wx),
                        sz * (1.0f - 2.0f * (xx + yy)), 0.0f,
                        Translation.x, Translation.y, Translation.z, 1.0f);
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
        Ref<class Mesh>         Mesh     = nullptr;
        Ref<class Material>     Material = nullptr;
        Ref<class TextureAsset> Texture  = nullptr;
        /// One material per material of the mesh file (Submesh::MaterialIndex);
        /// when set, each submesh draws with its own instead of `Material`.
        /// Built at runtime (MeshMaterials), not serialized.
        Vector<Ref<class Material>> SubmeshMaterials;

        MeshComponent()
            : Material(Material::Create(Renderer::ShaderLibrary().Get("PBR")))
        {
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
            if (Material)
            {
                const auto& surface = Material->Surface;
                SerializeValue(archive, "albedo",
                               Math::Vec4(surface.Albedo[0], surface.Albedo[1],
                                          surface.Albedo[2], surface.Albedo[3]));
                archive.Write("metallic", surface.Metallic);
                archive.Write("roughness", surface.Roughness);
            }
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
            if (!Material) Material = Material::Create(Renderer::ShaderLibrary().Get("PBR"));
            if (Texture) Material->Set("texSampler", Texture->Texture());

            // Surface constants; scenes saved before they were stored keep
            // the material's defaults.
            auto&      surface = Material->Surface;
            Math::Vec4 albedo(surface.Albedo[0], surface.Albedo[1], surface.Albedo[2],
                              surface.Albedo[3]);
            if (DeserializeValue(reader, "albedo", albedo))
                for (int i = 0; i < 4; ++i) surface.Albedo[i] = albedo[i];
            reader.Read("metallic", surface.Metallic);
            reader.Read("roughness", surface.Roughness);

            return true;
        }
    };
    /// Plays one of a skinned mesh's animation clips (the MeshComponent of
    /// the same actor). The scene advances it each update; the renderer skins
    /// the mesh with Pose.
    struct AnimationComponent
    {
        u32          Clip    = 0;
        f32          Time    = 0.0f; // seconds into the clip
        f32          Speed   = 1.0f;
        bool         Playing = true;
        /// Skinning matrices of the current pose, one per bone.
        Vector<Mat4> Pose;

        void         Serialize(ArchiveWriter& archive) const
        {
            archive.Write("clip", Clip);
            archive.Write("speed", Speed);
            archive.Write("playing", Playing);
        }
        bool Deserialize(ArchiveReader& reader)
        {
            if (!reader.Read("clip", Clip)) Clip = 0;
            if (!reader.Read("speed", Speed)) Speed = 1.0f;
            if (!reader.Read("playing", Playing)) Playing = true;
            return true;
        }
    };
    /// Makes the actor a PhysX body while the scene runs (see Vortex.Physics).
    /// Its shapes come from the actor's collider components.
    struct RigidBodyComponent
    {
        enum class BodyType : u8
        {
            /// Never moves (ground, walls).
            eStatic    = 0,
            /// Moved by the simulation.
            eDynamic   = 1,
            /// Moved by its transform; pushes dynamic bodies, isn't pushed.
            eKinematic = 2,
        };

        BodyType   Type           = BodyType::eDynamic;
        f32        Mass           = 1.0f;
        f32        LinearDamping  = 0.01f;
        f32        AngularDamping = 0.05f;
        bool       DisableGravity = false;
        /// Keeps the body upright (characters).
        bool       LockRotation   = false;

        // Runtime state, exchanged with the PhysicsScene on each Simulate.
        // Not serialized.
        /// Mirrors the body after each step.
        Math::Vec3 LinearVelocity{0.0f};
        /// Accumulated by gameplay, applied (and cleared) on the next step.
        Math::Vec3 PendingForce{0.0f};
        Math::Vec3 PendingImpulse{0.0f};
        /// LinearVelocity was set by gameplay and goes to the body next step.
        bool       VelocityDirty = false;
        /// Impulses at world-space points (e.g. bullet hits): they push and
        /// spin the body. Applied (and cleared) on the next step.
        struct PointImpulse
        {
            Math::Vec3 Impulse{0.0f};
            Math::Vec3 Position{0.0f};
        };
        Vector<PointImpulse> PendingPointImpulses;

        inline void SetLinearVelocity(const Math::Vec3& velocity)
        {
            LinearVelocity = velocity;
            VelocityDirty  = true;
        }
        inline void AddForce(const Math::Vec3& force) { PendingForce += force; }
        inline void AddImpulse(const Math::Vec3& impulse)
        {
            PendingImpulse += impulse;
        }
        inline void AddImpulseAtPosition(const Math::Vec3& impulse,
                                         const Math::Vec3& position)
        {
            PendingPointImpulses.PushBack({impulse, position});
        }

        void Serialize(ArchiveWriter& archive) const
        {
            archive.Write("type", static_cast<u32>(Type));
            archive.Write("mass", Mass);
            archive.Write("linearDamping", LinearDamping);
            archive.Write("angularDamping", AngularDamping);
            archive.Write("disableGravity", DisableGravity);
            archive.Write("lockRotation", LockRotation);
        }
        bool Deserialize(ArchiveReader& reader)
        {
            u32 type = static_cast<u32>(BodyType::eDynamic);
            reader.Read("type", type);
            Type = type <= static_cast<u32>(BodyType::eKinematic)
                     ? static_cast<BodyType>(type)
                     : BodyType::eDynamic;
            reader.Read("mass", Mass);
            reader.Read("linearDamping", LinearDamping);
            reader.Read("angularDamping", AngularDamping);
            reader.Read("disableGravity", DisableGravity);
            reader.Read("lockRotation", LockRotation);
            return true;
        }
    };

    /// Surface response shared by the colliders.
    struct ColliderMaterial
    {
        f32  Friction    = 0.6f;
        f32  Restitution = 0.1f;

        void Serialize(ArchiveWriter& archive) const
        {
            archive.Write("friction", Friction);
            archive.Write("restitution", Restitution);
        }
        void Deserialize(ArchiveReader& reader)
        {
            reader.Read("friction", Friction);
            reader.Read("restitution", Restitution);
        }
    };

    /// A box around the actor's origin, scaled by its transform.
    struct BoxColliderComponent
    {
        Math::Vec3       HalfExtents{0.5f};
        Math::Vec3       Offset{0.0f};
        ColliderMaterial Material;

        void             Serialize(ArchiveWriter& archive) const
        {
            SerializeValue(archive, "halfExtents", HalfExtents);
            SerializeValue(archive, "offset", Offset);
            Material.Serialize(archive);
        }
        bool Deserialize(ArchiveReader& reader)
        {
            DeserializeValue(reader, "halfExtents", HalfExtents);
            DeserializeValue(reader, "offset", Offset);
            Material.Deserialize(reader);
            return true;
        }
    };

    /// A sphere around the actor's origin, scaled by its largest axis.
    struct SphereColliderComponent
    {
        f32              Radius = 0.5f;
        Math::Vec3       Offset{0.0f};
        ColliderMaterial Material;

        void             Serialize(ArchiveWriter& archive) const
        {
            archive.Write("radius", Radius);
            SerializeValue(archive, "offset", Offset);
            Material.Serialize(archive);
        }
        bool Deserialize(ArchiveReader& reader)
        {
            reader.Read("radius", Radius);
            DeserializeValue(reader, "offset", Offset);
            Material.Deserialize(reader);
            return true;
        }
    };

    /// A capsule standing along the actor's local Y axis: `Height` is the
    /// total height, caps included. Scaled by the transform's X/Z (radius) and
    /// Y (height).
    struct CapsuleColliderComponent
    {
        f32              Radius = 0.3f;
        f32              Height = 1.8f;
        Math::Vec3       Offset{0.0f};
        ColliderMaterial Material;

        void             Serialize(ArchiveWriter& archive) const
        {
            archive.Write("radius", Radius);
            archive.Write("height", Height);
            SerializeValue(archive, "offset", Offset);
            Material.Serialize(archive);
        }
        bool Deserialize(ArchiveReader& reader)
        {
            reader.Read("radius", Radius);
            reader.Read("height", Height);
            DeserializeValue(reader, "offset", Offset);
            Material.Deserialize(reader);
            return true;
        }
    };

    /// Collides with the triangles of the actor's MeshComponent (scaled by its
    /// transform). Static only: level geometry such as Sponza.
    struct MeshColliderComponent
    {
        ColliderMaterial Material;

        void             Serialize(ArchiveWriter& archive) const
        {
            Material.Serialize(archive);
        }
        bool Deserialize(ArchiveReader& reader)
        {
            Material.Deserialize(reader);
            return true;
        }
    };

    /// A ball joint from this actor's body to `Connected`'s, at a world-space
    /// anchor (as placed when the scene starts). It twists around `Axis`
    /// within ±TwistLimit and swings away from it within SwingLimit degrees:
    /// ragdoll shoulders, hips, knees...
    struct JointComponent
    {
        entt::entity Connected = entt::null;
        Math::Vec3   Anchor{0.0f};
        Math::Vec3   Axis{0.0f, 1.0f, 0.0f};
        f32          SwingLimit = 45.0f;
        f32          TwistLimit = 20.0f;
    };

    /// A first-person character: WASD moves, Space jumps, Shift sprints, the
    /// mouse looks around (locked while playing; Escape frees it, a click
    /// takes it back). Needs a dynamic RigidBodyComponent with LockRotation
    /// and a collider. `Camera` is an actor with a CameraComponent placed at
    /// the eyes each frame. Driven by FirstPerson::Update (Play mode).
    struct FirstPersonControllerComponent
    {
        entt::entity Camera          = entt::null;
        f32          MoveSpeed       = 4.5f;
        f32          SprintMultiplier = 1.7f;
        f32          JumpSpeed       = 5.2f;
        f32          Acceleration    = 14.0f;
        /// Degrees per pixel of mouse motion.
        f32          LookSensitivity = 0.12f;
        /// Eye height above the actor's origin.
        f32          EyeHeight       = 0.65f;
        f32          FieldOfView     = 75.0f;

        // Runtime state.
        f32          Yaw        = 0.0f; // degrees, around +Y
        f32          Pitch      = 0.0f; // degrees, up is positive
        f32          JumpTimer  = 0.0f;
        f32          BobTime    = 0.0f;
        f32          BobAmount  = 0.0f;
        bool         Grounded   = false;
        bool         Started    = false;
    };

    /// A hitscan gun held by a FirstPersonControllerComponent's actor: the
    /// left mouse button fires along the view. Hits push dynamic bodies at
    /// the hit point and leave a bullet hole that sticks to what was hit.
    struct WeaponComponent
    {
        /// The gun's root actor; its child parts follow it (WeaponPartComponent).
        entt::entity       Model = entt::null;
        /// Shown for a moment at the muzzle on each shot.
        entt::entity       MuzzleFlash = entt::null;
        f32                FireRate    = 9.0f; // shots per second
        f32                Impulse     = 9.0f; // N·s per hit
        f32                Range       = 200.0f;
        f32                Spread      = 0.4f; // degrees, while moving
        u32                MaxDecals   = 160;
        /// The gun's rest position relative to the eyes (right, up, forward).
        Math::Vec3         HoldOffset{0.22f, -0.2f, 0.45f};
        Math::Vec3         MuzzleOffset{0.0f, 0.04f, 0.42f};
        ::Ref<class Mesh>     DecalMesh     = nullptr;
        ::Ref<class Material> DecalMaterial = nullptr;

        // Runtime state.
        f32                Cooldown = 0.0f;
        f32                Recoil   = 0.0f;
        f32                FlashTime = 0.0f;
        u32                ShotsFired = 0;
        Vector<entt::entity> Decals;
    };

    /// A piece of a weapon model, placed relative to the weapon's root.
    struct WeaponPartComponent
    {
        entt::entity Root = entt::null;
        /// Pose relative to the gun's root (x right, y up, -z forward).
        Math::Vec3   LocalPosition{0.0f};
        Quat         LocalRotation{1.0f, 0.0f, 0.0f, 0.0f};
        Math::Vec3   LocalScale{1.0f};
    };

    /// A bullet hole: follows `Target` (a moving body keeps its holes).
    struct BulletDecalComponent
    {
        entt::entity Target = entt::null;
        /// Pose in the target's space, without the target's scale.
        Math::Vec3   LocalPosition{0.0f};
        Quat         LocalRotation{1.0f, 0.0f, 0.0f, 0.0f};
        Math::Vec3   Size{0.06f, 0.06f, 0.01f};
    };

    /// Types a script's public field can have and be edited as.
    enum class ScriptFieldType : u8
    {
        eNone,
        eBool,
        eInt32,
        eUInt32,
        eInt64,
        eFloat,
        eDouble,
        eVector2,
        eVector3,
        eVector4,
        eString,
    };

    /// Bytes a field of `type` takes in a ScriptFieldValue (and in the
    /// managed object; bools marshal as 32-bit). 0 for strings.
    constexpr usize ScriptFieldSize(ScriptFieldType type)
    {
        switch (type)
        {
            case ScriptFieldType::eBool:
            case ScriptFieldType::eInt32:
            case ScriptFieldType::eUInt32:
            case ScriptFieldType::eFloat: return 4;
            case ScriptFieldType::eInt64:
            case ScriptFieldType::eDouble:
            case ScriptFieldType::eVector2: return 8;
            case ScriptFieldType::eVector3: return 12;
            case ScriptFieldType::eVector4: return 16;
            default: return 0;
        }
    }

    /// An edited value of a script's public field: set on the instance
    /// before its OnCreate, so each actor can tune its own copy.
    struct ScriptFieldValue
    {
        String          Name{};
        ScriptFieldType Type = ScriptFieldType::eNone;
        /// Numbers and vectors (bools as a 32-bit 0/1).
        alignas(8) u8   Data[16]{};
        /// Strings.
        String          Text{};

        template <typename T>
        inline T Get() const
        {
            static_assert(sizeof(T) <= sizeof(Data));
            T value{};
            std::memcpy(&value, Data, sizeof(T));
            return value;
        }
        template <typename T>
        inline void Set(const T& value)
        {
            static_assert(sizeof(T) <= sizeof(Data));
            std::memcpy(Data, &value, sizeof(T));
        }

        void Serialize(ArchiveWriter& archive) const
        {
            archive.Write("name", Name.View());
            archive.Write("type", static_cast<i32>(Type));
            switch (Type)
            {
                case ScriptFieldType::eBool: archive.Write("value", Get<i32>() != 0); break;
                case ScriptFieldType::eInt32: archive.Write("value", Get<i32>()); break;
                case ScriptFieldType::eUInt32: archive.Write("value", Get<u32>()); break;
                case ScriptFieldType::eInt64: archive.Write("value", Get<i64>()); break;
                case ScriptFieldType::eFloat: archive.Write("value", Get<f32>()); break;
                case ScriptFieldType::eDouble: archive.Write("value", Get<f64>()); break;
                case ScriptFieldType::eString: archive.Write("value", Text.View()); break;
                case ScriptFieldType::eVector2:
                case ScriptFieldType::eVector3:
                case ScriptFieldType::eVector4:
                {
                    f32 components[4]{};
                    std::memcpy(components, Data, ScriptFieldSize(Type));
                    usize count = ScriptFieldSize(Type) / sizeof(f32);
                    constexpr const char* kKeys[] = {"x", "y", "z", "w"};
                    for (usize i = 0; i < count; ++i) archive.Write(kKeys[i], components[i]);
                    break;
                }
                default: break;
            }
        }
        bool Deserialize(ArchiveReader& reader)
        {
            i32 type = 0;
            if (!reader.Read("name", Name) || !reader.Read("type", type)) return false;
            if (type <= 0 || type > static_cast<i32>(ScriptFieldType::eString)) return false;
            Type = static_cast<ScriptFieldType>(type);
            switch (Type)
            {
                case ScriptFieldType::eBool:
                {
                    bool value = false;
                    reader.Read("value", value);
                    Set<i32>(value ? 1 : 0);
                    break;
                }
                case ScriptFieldType::eInt32: { i32 v = 0; reader.Read("value", v); Set(v); break; }
                case ScriptFieldType::eUInt32: { u32 v = 0; reader.Read("value", v); Set(v); break; }
                case ScriptFieldType::eInt64: { i64 v = 0; reader.Read("value", v); Set(v); break; }
                case ScriptFieldType::eFloat: { f32 v = 0; reader.Read("value", v); Set(v); break; }
                case ScriptFieldType::eDouble: { f64 v = 0; reader.Read("value", v); Set(v); break; }
                case ScriptFieldType::eString: reader.Read("value", Text); break;
                default:
                {
                    f32 components[4]{};
                    constexpr const char* kKeys[] = {"x", "y", "z", "w"};
                    usize count = ScriptFieldSize(Type) / sizeof(f32);
                    for (usize i = 0; i < count; ++i) reader.Read(kKeys[i], components[i]);
                    std::memcpy(Data, components, ScriptFieldSize(Type));
                    break;
                }
            }
            return true;
        }
    };

    struct ScriptComponent
    {
        /// Full name of a Vortex.Actor subclass in the app assembly.
        String                   ClassName = "Gameplay.PlayerController";
        /// Public fields edited away from the class's defaults.
        Vector<ScriptFieldValue> Fields;

        ScriptComponent() = default;
        ScriptComponent(StringView className)
            : ClassName(className)
        {
        }
        ScriptComponent(const char* className)
            : ClassName(className)
        {
        }

        inline ScriptFieldValue* FindField(StringView name)
        {
            for (auto& field : Fields)
                if (field.Name == name) return &field;
            return nullptr;
        }
        inline const ScriptFieldValue* FindField(StringView name) const
        {
            for (const auto& field : Fields)
                if (field.Name == name) return &field;
            return nullptr;
        }
        inline void ResetField(StringView name)
        {
            for (usize i = 0; i < Fields.Size(); ++i)
            {
                if (Fields[i].Name != name) continue;
                Fields.Erase(Fields.begin() + i);
                return;
            }
        }

        void Serialize(ArchiveWriter& archive) const
        {
            archive.Write("className", ClassName.View());
            archive.BeginArray("fields", Fields.Size());
            for (const auto& field : Fields)
            {
                archive.BeginObject("");
                field.Serialize(archive);
                archive.EndObject();
            }
            archive.EndArray();
        }
        bool Deserialize(ArchiveReader& reader)
        {
            if (!reader.Read("className", ClassName)) return false;
            Fields.Clear();
            usize count = 0;
            if (reader.BeginArray("fields", count))
            {
                for (usize i = 0; i < count; ++i)
                {
                    if (!reader.BeginObject("")) continue;
                    ScriptFieldValue field;
                    if (field.Deserialize(reader)) Fields.PushBack(Move(field));
                    reader.EndObject();
                }
                reader.EndArray();
            }
            return true;
        }
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
