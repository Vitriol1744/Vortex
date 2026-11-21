/*
 * Created by v1tr10l7 on 18.11.2026.
 * Copyright (c) 2024-2026, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#include <Vortex/Engine/Application.hpp>
#include <Vortex/Engine/RuntimeAssetManager.hpp>
#include <Vortex/Renderer/Mesh.hpp>
#include <Vortex/Renderer/TextureAsset.hpp>
#include <Vortex/Scene/Actor.hpp>
#include <Vortex/Scene/Components.hpp>
#include <Vortex/Scene/Scene.hpp>
#include <Vortex/Scene/SceneRenderer.hpp>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

import Nexus.TestScenes;
import Prism.Containers.Vector;
import Vortex.Renderer.Material;

namespace Vortex::Editor
{
    namespace
    {
        constexpr PathView kCubeMesh     = "assets/models/cube.obj";
        constexpr PathView kSphereMesh   = "assets/models/sphere.obj";
        constexpr PathView kCylinderMesh = "assets/models/cylinder.obj";
        constexpr PathView kDiscMesh     = "assets/models/disc.obj";
        constexpr PathView kSponzaMesh   = "assets/models/sponza/Sponza.gltf";
        /// Sponza.gltf's root node scale (our loader doesn't apply nodes).
        constexpr f32      kSponzaScale  = 0.008f;

        struct Color
        {
            f32 R, G, B;
        };

        RuntimeAssetManager& Assets()
        {
            // The application always creates a RuntimeAssetManager.
            return static_cast<RuntimeAssetManager&>(*Application::Get()->AssetManager());
        }

        /// Meshes a scene builder loaded up front: AddShape uses these
        /// directly instead of loading in the background, so the actors are
        /// complete when the scene is copied for Play right away.
        struct PreloadedShape
        {
            PathView    Path;
            ::Ref<Mesh> Mesh;
        };
        Vector<PreloadedShape> s_PreloadedMeshes;

        ::Ref<Mesh> PreloadedMesh(PathView path)
        {
            for (const auto& preloaded : s_PreloadedMeshes)
                if (preloaded.Path == path) return preloaded.Mesh;
            return nullptr;
        }

        /// An actor showing `mesh` (once it loaded) in a flat color.
        Actor AddShape(const ::Ref<Scene>& scene, StringView name, PathView mesh,
                       const Math::Vec3& position, const Math::Vec3& scale, Color color,
                       f32 roughness = 0.6f)
        {
            Actor actor = scene->AddActor();
            actor.AddComponent<TagComponent>(String(name));
            auto& transform       = actor.AddComponent<TransformComponent>();
            transform.Translation = position;
            transform.Scale       = scale;

            auto& component = actor.AddComponent<MeshComponent>(
                nullptr, Material::Create(SceneRenderer::Shader()), nullptr);
            auto& surface             = component.Material->Surface;
            surface.Albedo[0]         = color.R;
            surface.Albedo[1]         = color.G;
            surface.Albedo[2]         = color.B;
            surface.Roughness         = roughness;

            if (auto preloaded = PreloadedMesh(mesh))
            {
                component.Mesh = preloaded;
                return actor;
            }

            // The scene ref keeps it alive until the mesh arrives.
            entt::entity entity = actor;
            Assets().LoadMeshAsync(mesh).Then(
                [scene, entity](const ::Ref<Mesh>& loaded)
                {
                    auto& registry = scene->Registry();
                    if (!loaded || !registry.valid(entity)) return;
                    if (auto* target = registry.try_get<MeshComponent>(entity))
                        target->Mesh = loaded;
                });
            return actor;
        }

        void AddStaticBox(const ::Ref<Scene>& scene, StringView name,
                          const Math::Vec3& position, const Math::Vec3& scale,
                          Color color, const Quat& rotation = Quat(1, 0, 0, 0))
        {
            Actor actor = AddShape(scene, name, kCubeMesh, position, scale, color, 0.9f);
            actor.GetComponent<TransformComponent>().Rotation = rotation;
            actor.AddComponent<RigidBodyComponent>().Type
                = RigidBodyComponent::BodyType::eStatic;
            actor.AddComponent<BoxColliderComponent>();
        }

        void AddCrate(const ::Ref<Scene>& scene, const Math::Vec3& position, Color color)
        {
            Actor actor = AddShape(scene, "Crate", kCubeMesh, position, Math::Vec3(1.0f),
                                   color);
            actor.AddComponent<RigidBodyComponent>().Mass = 2.0f;
            actor.AddComponent<BoxColliderComponent>();
        }

        void AddBall(const ::Ref<Scene>& scene, const Math::Vec3& position, f32 diameter,
                     Color color)
        {
            Actor actor = AddShape(scene, "Ball", kSphereMesh, position,
                                   Math::Vec3(diameter), color, 0.25f);
            actor.AddComponent<RigidBodyComponent>().Mass = diameter;
            auto& sphere                = actor.AddComponent<SphereColliderComponent>();
            sphere.Material.Restitution = 0.6f;
        }
        ::Ref<Material> FlatMaterial(Color color, f32 roughness, f32 metallic = 0.0f)
        {
            auto material                = Material::Create(SceneRenderer::Shader());
            material->Surface.Albedo[0]  = color.R;
            material->Surface.Albedo[1]  = color.G;
            material->Surface.Albedo[2]  = color.B;
            material->Surface.Roughness  = roughness;
            material->Surface.Metallic   = metallic;
            return material;
        }

        /// One material per material of the file, textures streaming in.
        Vector<::Ref<Material>> LoadMeshMaterials(const Mesh& mesh)
        {
            Vector<::Ref<Material>> materials;
            for (const auto& info : mesh.Materials())
            {
                auto material = Material::Create(SceneRenderer::Shader());
                for (int i = 0; i < 4; ++i) material->Surface.Albedo[i] = info.Albedo[i];
                // Metal/roughness maps aren't read: keep surfaces dielectric.
                material->Surface.Metallic  = 0.0f;
                material->Surface.Roughness = 0.85f;
                materials.PushBack(material);
                if (info.AlbedoTexture.Empty()) continue;
                Assets().LoadTexture2DAsync(info.AlbedoTexture)
                    .Then(
                        [material](const ::Ref<TextureAsset>& texture)
                        {
                            if (texture) material->Set("texSampler", texture->Texture());
                        });
            }
            return materials;
        }

        /// A piece of the gun: `local` places it relative to the gun's root
        /// (x right, y up, -z forward), in meters.
        void AddGunPart(const ::Ref<Scene>& scene, entt::entity root, StringView name,
                        const ::Ref<Mesh>& mesh, const ::Ref<Material>& material,
                        const Math::Vec3& position, const Math::Vec3& size,
                        const Quat& rotation = Quat(1, 0, 0, 0))
        {
            Actor part = scene->AddActor();
            part.AddComponent<TagComponent>(String(name));
            part.AddComponent<TransformComponent>();
            part.AddComponent<MeshComponent>(mesh, material, nullptr);
            auto& piece = part.AddComponent<WeaponPartComponent>();
            piece.Root  = root;
            piece.LocalPosition = position;
            piece.LocalRotation = rotation;
            piece.LocalScale    = size;
        }

        /// A compact rifle built from boxes and cylinders.
        entt::entity BuildRifle(const ::Ref<Scene>& scene, const ::Ref<Mesh>& cube,
                                const ::Ref<Mesh>& cylinder)
        {
            Actor root = scene->AddActor();
            root.AddComponent<TagComponent>("Rifle");
            root.AddComponent<TransformComponent>();
            entt::entity id = root;

            auto metal   = FlatMaterial({0.10f, 0.10f, 0.11f}, 0.32f, 0.85f);
            auto polymer = FlatMaterial({0.16f, 0.17f, 0.18f}, 0.65f);
            auto accent  = FlatMaterial({0.95f, 0.45f, 0.08f}, 0.45f);
            Quat alongZ  = glm::angleAxis(glm::radians(90.0f), Math::Vec3(1, 0, 0));
            auto tilt    = [](f32 degrees)
            { return glm::angleAxis(glm::radians(degrees), Math::Vec3(1, 0, 0)); };

            AddGunPart(scene, id, "Receiver", cube, polymer, {0.0f, 0.02f, -0.08f},
                       {0.07f, 0.09f, 0.32f});
            AddGunPart(scene, id, "Handguard", cube, metal, {0.0f, 0.03f, -0.30f},
                       {0.062f, 0.065f, 0.20f});
            AddGunPart(scene, id, "Barrel", cylinder, metal, {0.0f, 0.04f, -0.46f},
                       {0.024f, 0.20f, 0.024f}, alongZ);
            AddGunPart(scene, id, "Muzzle", cylinder, metal, {0.0f, 0.04f, -0.565f},
                       {0.036f, 0.035f, 0.036f}, alongZ);
            AddGunPart(scene, id, "Grip", cube, polymer, {0.0f, -0.075f, 0.02f},
                       {0.045f, 0.12f, 0.06f}, tilt(-18.0f));
            AddGunPart(scene, id, "Magazine", cube, metal, {0.0f, -0.085f, -0.13f},
                       {0.04f, 0.14f, 0.065f}, tilt(10.0f));
            AddGunPart(scene, id, "Stock", cube, polymer, {0.0f, 0.0f, 0.15f},
                       {0.05f, 0.08f, 0.16f});
            AddGunPart(scene, id, "Rail", cube, metal, {0.0f, 0.075f, -0.10f},
                       {0.026f, 0.016f, 0.24f});
            AddGunPart(scene, id, "Rear Sight", cube, metal, {0.0f, 0.095f, 0.0f},
                       {0.03f, 0.03f, 0.02f});
            AddGunPart(scene, id, "Front Sight", cube, metal, {0.0f, 0.085f, -0.37f},
                       {0.01f, 0.04f, 0.012f});
            AddGunPart(scene, id, "Stripe", cube, accent, {0.0f, 0.045f, -0.08f},
                       {0.072f, 0.012f, 0.12f});
            return id;
        }

        /// A box (or sphere) body of a ragdoll.
        entt::entity AddRagdollPart(const ::Ref<Scene>& scene, StringView name,
                                    const Math::Vec3& position, const Math::Vec3& size,
                                    Color color, f32 mass, bool sphere = false)
        {
            Actor part = AddShape(scene, name, sphere ? kSphereMesh : kCubeMesh, position,
                                  size, color, 0.7f);
            auto& body          = part.AddComponent<RigidBodyComponent>();
            body.Mass           = mass;
            body.LinearDamping  = 0.05f;
            body.AngularDamping = 0.6f;
            if (sphere) part.AddComponent<SphereColliderComponent>();
            else part.AddComponent<BoxColliderComponent>();
            return part;
        }

        void Connect(const ::Ref<Scene>& scene, entt::entity child, entt::entity parent,
                     const Math::Vec3& anchor, f32 swing, f32 twist,
                     const Math::Vec3& axis = {0.0f, 1.0f, 0.0f})
        {
            auto& joint      = scene->Registry().emplace<JointComponent>(child);
            joint.Connected  = parent;
            joint.Anchor     = anchor;
            joint.Axis       = axis;
            joint.SwingLimit = swing;
            joint.TwistLimit = twist;
        }

        /// A box mannequin standing with its feet at `feet`, its bodies
        /// joined at the neck, shoulders, elbows, hips and knees.
        void AddRagdoll(const ::Ref<Scene>& scene, const Math::Vec3& feet, Color shirt)
        {
            constexpr Color kSkin  = {0.78f, 0.62f, 0.5f};
            constexpr Color kPants = {0.18f, 0.2f, 0.28f};
            auto at = [&](f32 x, f32 y, f32 z) { return feet + Math::Vec3(x, y, z); };

            auto pelvis = AddRagdollPart(scene, "Ragdoll Pelvis", at(0, 0.98f, 0),
                                         {0.32f, 0.18f, 0.18f}, kPants, 10.0f);
            auto chest  = AddRagdollPart(scene, "Ragdoll Chest", at(0, 1.31f, 0),
                                         {0.36f, 0.42f, 0.2f}, shirt, 15.0f);
            auto head   = AddRagdollPart(scene, "Ragdoll Head", at(0, 1.66f, 0),
                                         {0.24f, 0.24f, 0.24f}, kSkin, 4.0f, true);
            Connect(scene, chest, pelvis, at(0, 1.08f, 0), 25.0f, 15.0f);
            Connect(scene, head, chest, at(0, 1.54f, 0), 35.0f, 30.0f);

            for (f32 side : {-1.0f, 1.0f})
            {
                f32  x        = side * 0.25f;
                auto upperArm = AddRagdollPart(scene, "Ragdoll Upper Arm", at(x, 1.33f, 0),
                                               {0.09f, 0.3f, 0.09f}, shirt, 2.5f);
                auto lowerArm = AddRagdollPart(scene, "Ragdoll Forearm", at(x, 1.03f, 0),
                                               {0.08f, 0.28f, 0.08f}, kSkin, 1.8f);
                Connect(scene, upperArm, chest, at(x, 1.49f, 0), 70.0f, 40.0f);
                Connect(scene, lowerArm, upperArm, at(x, 1.18f, 0), 60.0f, 10.0f);

                f32  hip   = side * 0.1f;
                auto thigh = AddRagdollPart(scene, "Ragdoll Thigh", at(hip, 0.67f, 0),
                                            {0.13f, 0.42f, 0.13f}, kPants, 7.0f);
                auto shin  = AddRagdollPart(scene, "Ragdoll Shin", at(hip, 0.23f, 0),
                                            {0.11f, 0.42f, 0.11f}, kPants, 4.0f);
                Connect(scene, thigh, pelvis, at(hip, 0.89f, 0), 45.0f, 15.0f);
                Connect(scene, shin, thigh, at(hip, 0.45f, 0), 50.0f, 5.0f);
            }
        }
    } // namespace

    ::Ref<Scene> CreateFpsTestScene()
    {
        auto scene = CreateRef<Scene>("FPS Test");

        // The props are small; load them now (the scene is played straight
        // away, before background loads queued behind Sponza's textures
        // would finish).
        s_PreloadedMeshes.Clear();
        for (PathView path : {kCubeMesh, kSphereMesh, kCylinderMesh, kDiscMesh})
            s_PreloadedMeshes.PushBack({path, CreateRef<Mesh>(path)});

        // The level: drawn per material, collided with per triangle.
        auto sponzaMesh = CreateRef<Mesh>(kSponzaMesh);
        {
            Actor sponza = scene->AddActor();
            sponza.AddComponent<TagComponent>("Sponza");
            sponza.AddComponent<TransformComponent>().Scale = Math::Vec3(kSponzaScale);
            auto& mesh = sponza.AddComponent<MeshComponent>(
                sponzaMesh, Material::Create(SceneRenderer::Shader()), nullptr);
            mesh.SubmeshMaterials = LoadMeshMaterials(*sponzaMesh);
            sponza.AddComponent<MeshColliderComponent>();
        }

        // Things to shoot, on the courtyard floor (y = 0), clear of the
        // potted plants in its corners.
        constexpr Color kCrateColors[]
            = {{0.8f, 0.45f, 0.2f}, {0.75f, 0.6f, 0.25f}, {0.6f, 0.35f, 0.2f}};
        for (int row = 0; row < 4; ++row)
            for (int column = 0; column < 4 - row; ++column)
            {
                f32 z = -1.2f + (column - (4 - row - 1) * 0.5f) * 1.02f;
                AddCrate(scene, {2.5f, 0.5f + row * 1.01f, z}, kCrateColors[(row + column) % 3]);
            }
        // A tower of small cubes.
        for (int i = 0; i < 7; ++i)
        {
            Actor cube = AddShape(scene, "Cube", kCubeMesh, {0.5f, 0.25f + i * 0.505f, 1.6f},
                                  Math::Vec3(0.5f), {0.25f, 0.55f, 0.85f}, 0.4f);
            cube.AddComponent<RigidBodyComponent>().Mass = 1.0f;
            cube.AddComponent<BoxColliderComponent>();
        }
        AddBall(scene, {4.0f, 0.5f, 1.4f}, 0.8f, {0.85f, 0.2f, 0.25f});
        AddBall(scene, {3.6f, 0.4f, -2.2f}, 0.6f, {0.2f, 0.75f, 0.3f});
        AddBall(scene, {-0.5f, 0.35f, -1.8f}, 0.5f, {0.95f, 0.85f, 0.2f});

        AddRagdoll(scene, {-1.5f, 0.02f, -0.8f}, {0.7f, 0.15f, 0.15f});
        AddRagdoll(scene, {-3.5f, 0.02f, 1.1f}, {0.15f, 0.45f, 0.75f});
        AddRagdoll(scene, {-5.5f, 0.02f, -0.3f}, {0.2f, 0.6f, 0.3f});

        // The player: a capsule looking down the courtyard (-X), its camera
        // and its rifle.
        auto cube     = PreloadedMesh(kCubeMesh);
        auto sphere   = PreloadedMesh(kSphereMesh);
        auto cylinder = PreloadedMesh(kCylinderMesh);

        Actor camera = scene->AddActor();
        camera.AddComponent<TagComponent>("Player Camera");
        camera.AddComponent<TransformComponent>();
        camera.AddComponent<CameraComponent>();

        Actor flash = scene->AddActor();
        flash.AddComponent<TagComponent>("Muzzle Flash");
        flash.AddComponent<TransformComponent>().Scale = Math::Vec3(0.0f);
        flash.AddComponent<MeshComponent>(sphere, FlatMaterial({6.0f, 3.6f, 1.0f}, 1.0f),
                                          nullptr);

        Actor player = scene->AddActor();
        player.AddComponent<TagComponent>("Player");
        auto& transform       = player.AddComponent<TransformComponent>();
        transform.Translation = {9.0f, 1.0f, 0.0f};
        transform.Rotation    = glm::angleAxis(glm::radians(90.0f), Math::Vec3(0, 1, 0));
        auto& body            = player.AddComponent<RigidBodyComponent>();
        body.Mass             = 75.0f;
        body.LockRotation     = true;
        body.LinearDamping    = 0.0f;
        auto& capsule         = player.AddComponent<CapsuleColliderComponent>();
        capsule.Radius        = 0.3f;
        capsule.Height        = 1.8f;
        // No friction against walls; the controller steers the velocity.
        capsule.Material.Friction    = 0.0f;
        capsule.Material.Restitution = 0.0f;

        auto& controller     = player.AddComponent<FirstPersonControllerComponent>();
        controller.Camera    = camera;
        controller.EyeHeight = 0.72f;

        auto& weapon         = player.AddComponent<WeaponComponent>();
        weapon.Model         = BuildRifle(scene, cube, cylinder);
        weapon.MuzzleFlash   = flash;
        weapon.MuzzleOffset  = {0.0f, 0.04f, 0.6f};
        // A flat disc: holes are many and small, so they stay cheap.
        weapon.DecalMesh     = PreloadedMesh(kDiscMesh);
        weapon.DecalMaterial = FlatMaterial({0.035f, 0.03f, 0.028f}, 0.95f);
        // The actors hold their own references now.
        s_PreloadedMeshes.Clear();
        return scene;
    }

    ::Ref<Scene> CreatePhysicsTestScene(bool preloadMeshes)
    {
        auto scene = CreateRef<Scene>("Physics Test");
        s_PreloadedMeshes.Clear();
        if (preloadMeshes)
            for (PathView path : {kCubeMesh, kSphereMesh})
                s_PreloadedMeshes.PushBack({path, CreateRef<Mesh>(path)});

        AddStaticBox(scene, "Ground", {0.0f, -0.25f, 0.0f}, {30.0f, 0.5f, 30.0f},
                     {0.35f, 0.37f, 0.4f});
        AddStaticBox(scene, "Ramp", {-6.0f, 1.0f, -2.0f}, {6.0f, 0.3f, 3.0f},
                     {0.55f, 0.45f, 0.35f},
                     glm::angleAxis(glm::radians(-20.0f), Math::Vec3(0.0f, 0.0f, 1.0f)));
        AddStaticBox(scene, "Wall", {0.0f, 1.0f, -10.0f}, {20.0f, 2.0f, 0.5f},
                     {0.4f, 0.4f, 0.45f});
        // Low borders along the ground's edges keep what rolls off the ramp
        // (or gets pushed) from falling forever.
        constexpr Color kBorder = {0.3f, 0.32f, 0.36f};
        AddStaticBox(scene, "Border", {15.25f, 0.5f, 0.0f}, {0.5f, 1.0f, 31.0f}, kBorder);
        AddStaticBox(scene, "Border", {-15.25f, 0.5f, 0.0f}, {0.5f, 1.0f, 31.0f}, kBorder);
        AddStaticBox(scene, "Border", {0.0f, 0.5f, 15.25f}, {30.0f, 1.0f, 0.5f}, kBorder);
        AddStaticBox(scene, "Border", {0.0f, 0.5f, -15.25f}, {30.0f, 1.0f, 0.5f}, kBorder);

        // A pyramid of crates, slightly apart so it settles instead of
        // starting interpenetrated.
        constexpr Color kCrateColors[]
            = {{0.8f, 0.45f, 0.2f}, {0.75f, 0.6f, 0.25f}, {0.6f, 0.35f, 0.2f}};
        constexpr int kBase = 5;
        for (int row = 0; row < kBase; ++row)
            for (int column = 0; column < kBase - row; ++column)
            {
                f32 x = 3.0f + (column - (kBase - row - 1) * 0.5f) * 1.02f;
                f32 y = 0.5f + row * 1.01f;
                AddCrate(scene, {x, y, -2.0f}, kCrateColors[(row + column) % 3]);
            }

        // Balls dropped on the ramp and over the pyramid.
        AddBall(scene, {-8.0f, 5.0f, -2.0f}, 1.0f, {0.85f, 0.2f, 0.25f});
        AddBall(scene, {-7.0f, 7.0f, -1.5f}, 0.7f, {0.2f, 0.75f, 0.3f});
        AddBall(scene, {-6.5f, 9.0f, -2.5f}, 0.5f, {0.95f, 0.85f, 0.2f});
        AddBall(scene, {3.0f, 10.0f, -2.0f}, 1.2f, {0.3f, 0.5f, 0.95f});

        // The player: upright box driven by its script.
        Actor player = AddShape(scene, "Player", kCubeMesh, {0.0f, 1.0f, 4.0f},
                                {0.8f, 1.6f, 0.8f}, {0.25f, 0.55f, 0.95f}, 0.4f);
        auto& body        = player.AddComponent<RigidBodyComponent>();
        body.Mass         = 5.0f;
        body.LockRotation = true;
        player.AddComponent<BoxColliderComponent>().Material.Friction = 0.2f;
        player.AddComponent<ScriptComponent>("Gameplay.PlayerController");

        // The actors hold their own references now.
        s_PreloadedMeshes.Clear();
        return scene;
    }
} // namespace Vortex::Editor
