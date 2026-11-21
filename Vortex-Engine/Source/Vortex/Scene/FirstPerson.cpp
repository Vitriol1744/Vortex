/*
 * Created by v1tr10l7 on 18.11.2026.
 * Copyright (c) 2024-2026, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#include <Vortex/Core/Profiler.hpp>
#include <Vortex/Engine/Application.hpp>
#include <Vortex/Renderer/RenderTargetWindow.hpp>
#include <Vortex/Scene/Actor.hpp>
#include <Vortex/Scene/Components.hpp>
#include <Vortex/Scene/FirstPerson.hpp>
#include <Vortex/Scene/Scene.hpp>
#include <Vortex/Window/Input/Keyboard.hpp>
#include <Vortex/Window/Input/Mouse.hpp>
#include <Vortex/Window/Window.hpp>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

#include <cmath>
#include <cstdlib> // TEMP-AUTOTEST

import Prism.Containers.Vector;
import Vortex.Physics;

namespace Vortex::FirstPerson
{
    namespace
    {
        using Input::KeyCode;
        using Input::MouseCode;

        constexpr Math::Vec3 kUp{0.0f, 1.0f, 0.0f};

        bool s_CaptureAllowed = true;
        bool s_WasEscape      = false;
        bool s_WasClick       = false;
        u32  s_RandomState    = 0x9E3779B9u;

        /// Uniform in [-1, 1].
        f32 Random()
        {
            s_RandomState = s_RandomState * 1664525u + 1013904223u;
            return static_cast<f32>(s_RandomState >> 8) / static_cast<f32>(1u << 23) - 1.0f;
        }

        Window* MainWindow()
        {
            auto* app = Application::Get();
            return app ? &app->Window().Window() : nullptr;
        }

        void SetCaptured(bool captured)
        {
            if (Window* window = MainWindow(); window && window->IsCursorLocked() != captured)
                window->SetCursorLocked(captured);
            // Motion from before the capture isn't a look.
            Mouse::ConsumeDelta();
        }

        /// Looking down -Z at yaw = pitch = 0; yaw turns left, pitch up.
        Quat ViewOrientation(f32 yaw, f32 pitch)
        {
            return glm::angleAxis(glm::radians(yaw), kUp)
                 * glm::angleAxis(glm::radians(pitch), Math::Vec3(1.0f, 0.0f, 0.0f));
        }

        /// Half the height of the actor's collider (for ground checks).
        f32 HalfHeight(Actor actor, const TransformComponent& transform)
        {
            f32 scaleY = std::abs(transform.Scale.y);
            if (actor.HasComponent<CapsuleColliderComponent>())
                return actor.GetComponent<CapsuleColliderComponent>().Height * scaleY * 0.5f;
            if (actor.HasComponent<BoxColliderComponent>())
                return actor.GetComponent<BoxColliderComponent>().HalfExtents.y * scaleY;
            if (actor.HasComponent<SphereColliderComponent>())
                return actor.GetComponent<SphereColliderComponent>().Radius * scaleY;
            return scaleY * 0.5f;
        }

        Math::Vec3 EyePosition(const FirstPersonControllerComponent& controller,
                               const TransformComponent&             transform)
        {
            // A little head bob while walking.
            f32 bob = std::sin(controller.BobTime * 2.0f) * 0.035f * controller.BobAmount;
            return transform.Translation + kUp * (controller.EyeHeight + bob);
        }

        struct Shot
        {
            entt::entity Shooter;
            Math::Vec3   Origin;
            Math::Vec3   Direction;
        };

        /// A bullet hole on `target` (or on nothing) at a hit point, facing
        /// out of the surface.
        void SpawnDecal(Scene& scene, WeaponComponent& weapon, entt::entity target,
                        const Math::Vec3& position, const Math::Vec3& normal)
        {
            auto& registry = scene.Registry();
            if (!weapon.DecalMesh) return;

            // Oldest holes are recycled past the limit.
            entt::entity decal = entt::null;
            while (!weapon.Decals.Empty() && weapon.Decals.Size() >= weapon.MaxDecals)
            {
                entt::entity oldest = weapon.Decals[0];
                weapon.Decals.Erase(weapon.Decals.begin());
                if (registry.valid(oldest))
                {
                    decal = oldest;
                    break;
                }
            }
            if (decal == entt::null)
            {
                Actor actor = scene.AddActor();
                actor.AddComponent<TagComponent>("Bullet Hole");
                actor.AddComponent<TransformComponent>();
                actor.AddComponent<MeshComponent>(weapon.DecalMesh, weapon.DecalMaterial,
                                                  nullptr);
                actor.AddComponent<BulletDecalComponent>();
                decal = actor;
            }
            weapon.Decals.PushBack(decal);

            Math::Vec3 n = glm::length(normal) > 0.0f ? glm::normalize(normal) : kUp;
            Quat world   = glm::rotation(Math::Vec3(0.0f, 0.0f, 1.0f), n)
                       * glm::angleAxis(Random() * glm::pi<f32>(), Math::Vec3(0, 0, 1));
            Math::Vec3 point = position + n * 0.004f;

            auto& hole = registry.get<BulletDecalComponent>(decal);
            hole.Target = target;
            f32 size    = 0.05f + 0.015f * (Random() * 0.5f + 0.5f);
            hole.Size   = {size, size, 0.008f};
            // Only holes on bodies that move need to follow them; the rest
            // (the level, static props) are placed once, here.
            const auto* body = target != entt::null && registry.valid(target)
                                 ? registry.try_get<RigidBodyComponent>(target)
                                 : nullptr;
            bool moves = body && body->Type != RigidBodyComponent::BodyType::eStatic;
            if (moves && registry.all_of<TransformComponent>(target))
            {
                const auto& base   = registry.get<TransformComponent>(target);
                Quat        invert = glm::inverse(base.Rotation);
                hole.LocalPosition = invert * (point - base.Translation);
                hole.LocalRotation = invert * world;
            }
            else
            {
                hole.Target        = entt::null;
                hole.LocalPosition = point;
                hole.LocalRotation = world;
            }

            auto& transform       = registry.get<TransformComponent>(decal);
            transform.Translation = point;
            transform.Rotation    = world;
            transform.Scale       = hole.Size;
        }

        void Fire(Scene& scene, const Shot& shot, WeaponComponent& weapon)
        {
            PhysicsScene* physics = scene.Physics();
            if (!physics) return;

            RaycastHit hit{};
            f32 origin[3]    = {shot.Origin.x, shot.Origin.y, shot.Origin.z};
            f32 direction[3] = {shot.Direction.x, shot.Direction.y, shot.Direction.z};
            if (!physics->Raycast(origin, direction, weapon.Range, hit,
                                  static_cast<u32>(entt::to_integral(shot.Shooter))))
                return;

            auto&        registry = scene.Registry();
            entt::entity target   = static_cast<entt::entity>(hit.Entity);
            Math::Vec3   point(hit.Position[0], hit.Position[1], hit.Position[2]);
            Math::Vec3   normal(hit.Normal[0], hit.Normal[1], hit.Normal[2]);

            if (registry.valid(target))
                if (auto* body = registry.try_get<RigidBodyComponent>(target);
                    body && body->Type == RigidBodyComponent::BodyType::eDynamic)
                    body->AddImpulseAtPosition(shot.Direction * weapon.Impulse, point);

            SpawnDecal(scene, weapon, registry.valid(target) ? target : entt::null, point,
                       normal);
        }
    } // namespace

    void SetCaptureAllowed(bool allowed) { s_CaptureAllowed = allowed; }
    bool IsMouseCaptured()
    {
        Window* window = MainWindow();
        return window && window->IsCursorLocked();
    }

    void Update(Scene& scene, f32 deltaTime)
    {
        VtProfileFunction();
        auto& registry = scene.Registry();
        auto  view     = registry.view<FirstPersonControllerComponent, TransformComponent,
                                       RigidBodyComponent>();
        if (view.begin() == view.end()) return;

        // Escape gives the mouse back; a click (where allowed) takes it.
        bool escape = Keyboard::IsKeyPressed(KeyCode::eEscape);
        bool click  = Mouse::IsButtonPressed(MouseCode::eLeft);
        bool locked = IsMouseCaptured();
        // The click that captures the mouse doesn't also shoot.
        bool firing = click && locked;
        if (escape && !s_WasEscape && locked) SetCaptured(false);
        else if (click && !s_WasClick && !locked && s_CaptureAllowed) SetCaptured(true);
        s_WasEscape = escape;
        s_WasClick  = click;
        locked      = IsMouseCaptured();

        Math::Vec2d look = Mouse::ConsumeDelta();
        // TEMP-AUTOTEST-BEGIN: scripted input for screenshots (no real mouse).
        if (const char* autotest = std::getenv("VT_AUTOTEST");
            autotest && StringView(autotest).StartsWith("fps") && StringView(autotest) != "fps-edit")
        {
            static int frame = 0;
            ++frame;
            locked = true;
            // Turn left and right across the courtyard while shooting bursts.
            f32 phase = static_cast<f32>(frame) * 0.012f;
            look      = Math::Vec2d(std::sin(phase) * 9.0, std::sin(phase * 0.7f) * 1.2);
            if (StringView(autotest) == "fps-still") look = Math::Vec2d(0.0, 0.0);
            firing    = frame > 60 && (frame / 25) % 2 == 0 && StringView(autotest) == "fps";
            if (frame % 120 == 0)
                for (auto entity : registry.view<WeaponComponent>())
                    VtCoreInfoTag("AUTOTEST", "frame {} shots {} decals {}", frame,
                                  registry.get<WeaponComponent>(entity).ShotsFired,
                                  registry.get<WeaponComponent>(entity).Decals.Size());
        }
        // TEMP-AUTOTEST-END
        Vector<Shot> shots;

        for (auto entity : view)
        {
            auto& controller = view.get<FirstPersonControllerComponent>(entity);
            auto& transform  = view.get<TransformComponent>(entity);
            auto& body       = view.get<RigidBodyComponent>(entity);
            Actor actor(entity, scene);

            if (!controller.Started)
            {
                controller.Started = true;
                // Start looking where the actor faces.
                Math::Vec3 forward = transform.Rotation * Math::Vec3(0.0f, 0.0f, -1.0f);
                controller.Yaw     = glm::degrees(std::atan2(-forward.x, -forward.z));
                controller.Pitch   = 0.0f;
            }

            if (locked)
            {
                controller.Yaw -= static_cast<f32>(look.x) * controller.LookSensitivity;
                controller.Pitch -= static_cast<f32>(look.y) * controller.LookSensitivity;
                controller.Pitch = glm::clamp(controller.Pitch, -89.0f, 89.0f);
                if (controller.Yaw > 360.0f || controller.Yaw < -360.0f)
                    controller.Yaw = std::fmod(controller.Yaw, 360.0f);
            }

            // Movement on the ground plane, relative to where we look.
            f32        yaw = glm::radians(controller.Yaw);
            Math::Vec3 forward(-std::sin(yaw), 0.0f, -std::cos(yaw));
            Math::Vec3 right(std::cos(yaw), 0.0f, -std::sin(yaw));
            Math::Vec3 input(0.0f);
            if (Keyboard::IsKeyPressed(KeyCode::eW)) input += forward;
            if (Keyboard::IsKeyPressed(KeyCode::eS)) input -= forward;
            if (Keyboard::IsKeyPressed(KeyCode::eD)) input += right;
            if (Keyboard::IsKeyPressed(KeyCode::eA)) input -= right;
            if (glm::length(input) > 0.0f) input = glm::normalize(input);

            f32 speed = controller.MoveSpeed;
            if (Keyboard::IsKeyPressed(KeyCode::eLShift)) speed *= controller.SprintMultiplier;

            // On the ground if a short ray down from the centre hits something.
            controller.Grounded = false;
            if (PhysicsScene* physics = scene.Physics())
            {
                RaycastHit hit{};
                f32 origin[3] = {transform.Translation.x, transform.Translation.y,
                                 transform.Translation.z};
                f32 down[3]   = {0.0f, -1.0f, 0.0f};
                controller.Grounded
                    = physics->Raycast(origin, down, HalfHeight(actor, transform) + 0.12f,
                                       hit, static_cast<u32>(entt::to_integral(entity)));
            }

            Math::Vec3 velocity = body.LinearVelocity;
            // Full control on the ground, a little in the air.
            f32 control = controller.Grounded ? controller.Acceleration
                                              : controller.Acceleration * 0.2f;
            f32 blend   = glm::clamp(control * deltaTime, 0.0f, 1.0f);
            velocity.x += (input.x * speed - velocity.x) * blend;
            velocity.z += (input.z * speed - velocity.z) * blend;

            controller.JumpTimer -= deltaTime;
            if (controller.Grounded && controller.JumpTimer <= 0.0f
                && Keyboard::IsKeyPressed(KeyCode::eSpace))
            {
                velocity.y           = controller.JumpSpeed;
                controller.JumpTimer = 0.3f;
            }
            body.SetLinearVelocity(velocity);

            // Head bob follows the walking speed.
            f32 planar = glm::length(Math::Vec2(velocity.x, velocity.z));
            f32 target = controller.Grounded && planar > 0.5f ? glm::min(planar / 6.0f, 1.0f) : 0.0f;
            controller.BobAmount += (target - controller.BobAmount) * glm::clamp(deltaTime * 8.0f, 0.0f, 1.0f);
            controller.BobTime += deltaTime * (3.0f + planar * 0.9f);

            // Shooting.
            auto* weapon = registry.try_get<WeaponComponent>(entity);
            if (!weapon) continue;
            weapon->Cooldown -= deltaTime;
            weapon->FlashTime -= deltaTime;
            weapon->Recoil = glm::max(0.0f, weapon->Recoil - deltaTime * 6.0f);
            if (!firing || weapon->Cooldown > 0.0f) continue;

            weapon->Cooldown = 1.0f / glm::max(weapon->FireRate, 0.1f);
            weapon->Recoil   = glm::min(weapon->Recoil + 1.0f, 2.5f);
            weapon->FlashTime = 0.05f;
            ++weapon->ShotsFired;
            // A small kick that stays, like a real gun's climb.
            controller.Pitch = glm::min(controller.Pitch + 0.35f, 89.0f);

            Quat       view = ViewOrientation(controller.Yaw, controller.Pitch);
            // Moving or airborne spreads the shots.
            f32 spread = weapon->Spread * (planar > 0.5f || !controller.Grounded ? 1.0f : 0.15f);
            Quat jitter = glm::angleAxis(glm::radians(Random() * spread), kUp)
                        * glm::angleAxis(glm::radians(Random() * spread),
                                         Math::Vec3(1.0f, 0.0f, 0.0f));
            shots.PushBack({entity, EyePosition(controller, transform),
                            glm::normalize(view * jitter * Math::Vec3(0.0f, 0.0f, -1.0f))});
        }

        // After the loop: shots add actors (bullet holes).
        for (const auto& shot : shots)
            if (auto* weapon = registry.try_get<WeaponComponent>(shot.Shooter))
                Fire(scene, shot, *weapon);
    }

    void LateUpdate(Scene& scene, f32 deltaTime)
    {
        VtProfileFunction();
        IgnoreUnused(deltaTime);
        auto& registry = scene.Registry();

        auto view = registry.view<FirstPersonControllerComponent, TransformComponent>();
        for (auto entity : view)
        {
            const auto& controller = view.get<FirstPersonControllerComponent>(entity);
            const auto& transform  = view.get<TransformComponent>(entity);
            Math::Vec3  eye        = EyePosition(controller, transform);
            Quat        look       = ViewOrientation(controller.Yaw, controller.Pitch);

            if (registry.valid(controller.Camera))
            {
                if (auto* cameraTransform = registry.try_get<TransformComponent>(controller.Camera))
                {
                    cameraTransform->Translation = eye;
                    cameraTransform->Rotation    = look;
                }
                if (auto* camera = registry.try_get<CameraComponent>(controller.Camera))
                {
                    camera->Camera.SetPosition(eye);
                    camera->Camera.SetRotation(glm::conjugate(look));
                }
            }

            auto* weapon = registry.try_get<WeaponComponent>(entity);
            if (!weapon || !registry.valid(weapon->Model)) continue;

            // The gun hangs at its hold offset (x right, y up, z forward),
            // sways with the head bob and kicks back on each shot.
            f32 recoil = weapon->Recoil;
            f32 swayX  = std::cos(controller.BobTime) * 0.012f * controller.BobAmount;
            f32 swayY  = -std::abs(std::sin(controller.BobTime)) * 0.010f * controller.BobAmount;
            Math::Vec3 local(weapon->HoldOffset.x + swayX, weapon->HoldOffset.y + swayY,
                             -weapon->HoldOffset.z + recoil * 0.035f);
            Quat kick = glm::angleAxis(glm::radians(recoil * 3.0f), Math::Vec3(1.0f, 0.0f, 0.0f));
            // The gun's root is a rigid pose (no scale), so every piece is
            // placed by composing rotations and offsets directly.
            const Quat       rootRotation = look * kick;
            const Math::Vec3 rootPosition = eye + look * local;
            auto place = [&](TransformComponent& target, const Math::Vec3& position,
                             const Quat& rotation, const Math::Vec3& scale)
            {
                target.Translation = rootPosition + rootRotation * position;
                target.Rotation    = rootRotation * rotation;
                target.Scale       = scale;
            };

            if (auto* model = registry.try_get<TransformComponent>(weapon->Model))
                place(*model, Math::Vec3(0.0f), Quat(1.0f, 0.0f, 0.0f, 0.0f), Math::Vec3(1.0f));
            for (auto part : registry.view<WeaponPartComponent, TransformComponent>())
            {
                const auto& piece = registry.get<WeaponPartComponent>(part);
                if (piece.Root != weapon->Model) continue;
                place(registry.get<TransformComponent>(part), piece.LocalPosition,
                      piece.LocalRotation, piece.LocalScale);
            }

            if (registry.valid(weapon->MuzzleFlash))
                if (auto* flash = registry.try_get<TransformComponent>(weapon->MuzzleFlash))
                {
                    Math::Vec3 muzzle(weapon->MuzzleOffset.x, weapon->MuzzleOffset.y,
                                      -weapon->MuzzleOffset.z);
                    f32 size = weapon->FlashTime > 0.0f ? 0.07f + Random() * 0.02f : 0.0f;
                    place(*flash, muzzle,
                          glm::angleAxis(Random() * 3.14f, Math::Vec3(0, 0, 1)),
                          Math::Vec3(size, size, size * 1.6f));
                }
        }

        // Bullet holes ride on whatever they hit.
        for (auto entity : registry.view<BulletDecalComponent, TransformComponent>())
        {
            const auto& hole = registry.get<BulletDecalComponent>(entity);
            if (hole.Target == entt::null || !registry.valid(hole.Target)) continue;
            auto& transform = registry.get<TransformComponent>(entity);
            const auto* base = registry.try_get<TransformComponent>(hole.Target);
            if (!base) continue;
            transform.Translation = base->Translation + base->Rotation * hole.LocalPosition;
            transform.Rotation    = base->Rotation * hole.LocalRotation;
        }
    }

    void Stop(Scene& scene)
    {
        IgnoreUnused(scene);
        if (IsMouseCaptured()) SetCaptured(false);
        s_WasEscape = false;
        s_WasClick  = false;
    }

    Camera* ActiveCamera(Scene& scene, f32* outFieldOfView)
    {
        auto& registry = scene.Registry();
        for (auto entity : registry.view<FirstPersonControllerComponent>())
        {
            const auto& controller = registry.get<FirstPersonControllerComponent>(entity);
            if (!registry.valid(controller.Camera)) continue;
            auto* camera = registry.try_get<CameraComponent>(controller.Camera);
            if (!camera) continue;
            if (outFieldOfView) *outFieldOfView = controller.FieldOfView;
            return &camera->Camera;
        }
        return nullptr;
    }
} // namespace Vortex::FirstPerson
