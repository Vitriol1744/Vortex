/*
 * Created by v1tr10l7 on 18.11.2026.
 * Copyright (c) 2024-2026, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#include <Vortex/Physics/PhysXInternal.hpp>

#include <Vortex/Core/Core.hpp>
#include <Vortex/Core/Profiler.hpp>
#include <Vortex/Core/Timer.hpp>
#include <Vortex/Scene/Actor.hpp>
#include <Vortex/Scene/Components.hpp>
#include <Vortex/Scene/Scene.hpp>

#include <glm/glm.hpp>

#include <cmath>
#include <cstdint>

import Prism.Containers.Vector;
import Vortex.Physics;

using namespace physx;

namespace Vortex
{
    namespace
    {
        using BodyType = RigidBodyComponent::BodyType;

        inline PxVec3 ToPx(const Math::Vec3& v) { return PxVec3(v.x, v.y, v.z); }
        inline PxQuat ToPx(const Quat& q) { return PxQuat(q.x, q.y, q.z, q.w); }
        inline Math::Vec3 FromPx(const PxVec3& v) { return {v.x, v.y, v.z}; }
        inline Quat FromPx(const PxQuat& q) { return Quat(q.w, q.x, q.y, q.z); }

        inline PxTransform ToPx(const TransformComponent& transform)
        {
            PxQuat rotation = ToPx(transform.Rotation);
            // Gizmo edits can leave a slightly denormalized quaternion, which
            // PhysX rejects.
            if (!rotation.isSane()) rotation = PxQuat(PxIdentity);
            return PxTransform(ToPx(transform.Translation), rotation.getNormalized());
        }

        inline void* EntityToUserData(entt::entity entity)
        {
            return reinterpret_cast<void*>(
                static_cast<std::uintptr_t>(entt::to_integral(entity)));
        }
        inline u32 UserDataToEntity(const void* data)
        {
            return static_cast<u32>(reinterpret_cast<std::uintptr_t>(data));
        }

        PxMaterial* CreateMaterial(const ColliderMaterial& material)
        {
            return PhysicsEngine::Internal::Physics().createMaterial(
                material.Friction, material.Friction, material.Restitution);
        }

        /// Attaches `geometry` to `actor` at `offset` (in the actor's scaled
        /// local space). The shape keeps the material alive.
        void AttachShape(PxRigidActor& actor, const PxGeometry& geometry,
                         const PxVec3& offset, const ColliderMaterial& material,
                         const PxQuat& rotation = PxQuat(PxIdentity))
        {
            PxMaterial* pxMaterial = CreateMaterial(material);
            PxShape*    shape
                = PxRigidActorExt::createExclusiveShape(actor, geometry, *pxMaterial);
            pxMaterial->release();
            if (shape) shape->setLocalPose(PxTransform(offset, rotation));
        }

        /// A rotation taking +X (PhysX's joint twist axis) onto `axis`.
        PxQuat RotationFromX(PxVec3 axis)
        {
            PxVec3 from(1.0f, 0.0f, 0.0f);
            if (axis.magnitudeSquared() < 1e-8f) return PxQuat(PxIdentity);
            axis.normalize();
            f32 dot = from.dot(axis);
            if (dot > 0.9999f) return PxQuat(PxIdentity);
            if (dot < -0.9999f) return PxQuat(PxPi, PxVec3(0.0f, 1.0f, 0.0f));
            PxVec3 cross = from.cross(axis);
            PxQuat q(cross.x, cross.y, cross.z, 1.0f + dot);
            return q.getNormalized();
        }

        /// Skips the actor casting the ray.
        class IgnoreActorFilter final : public PxQueryFilterCallback
        {
          public:
            explicit IgnoreActorFilter(u32 ignored)
                : m_Ignored(ignored)
            {
            }

            PxQueryHitType::Enum preFilter(const PxFilterData&, const PxShape*,
                                           const PxRigidActor* actor,
                                           PxHitFlags&) override
            {
                if (actor && UserDataToEntity(actor->userData) == m_Ignored)
                    return PxQueryHitType::eNONE;
                return PxQueryHitType::eBLOCK;
            }
            PxQueryHitType::Enum postFilter(const PxFilterData&, const PxQueryHit&,
                                            const PxShape*, const PxRigidActor*) override
            {
                return PxQueryHitType::eBLOCK;
            }

          private:
            u32 m_Ignored;
        };
    } // namespace

    struct PhysicsScene::Impl
    {
        struct Body
        {
            entt::entity   Entity  = entt::null;
            PxRigidActor*  Actor   = nullptr;
            /// Null for static actors.
            PxRigidDynamic* Dynamic = nullptr;
            bool           Kinematic = false;
        };

        Scene&       Owner;
        PxScene*     World       = nullptr;
        Vector<Body> Bodies;
        Vector<PxJoint*> Joints;
        /// Cooked once per render mesh, shared by every actor using it.
        struct CookedMesh
        {
            const Mesh*     Source   = nullptr;
            PxTriangleMesh* Triangles = nullptr;
        };
        Vector<CookedMesh> CookedMeshes;
        f32          Accumulator = 0.0f;

        explicit Impl(Scene& owner)
            : Owner(owner)
        {
        }

        void CreateWorld()
        {
            auto&       physics  = PhysicsEngine::Internal::Physics();
            const auto& settings = PhysicsEngine::Settings();

            PxSceneDesc desc(physics.getTolerancesScale());
            desc.gravity       = PxVec3(settings.Gravity[0], settings.Gravity[1],
                                        settings.Gravity[2]);
            desc.cpuDispatcher = &PhysicsEngine::Internal::Dispatcher();
            desc.filterShader  = PxDefaultSimulationFilterShader;
            World              = physics.createScene(desc);
        }

        void CreateBody(Actor actor)
        {
            auto&       physics   = PhysicsEngine::Internal::Physics();
            const auto& transform = actor.GetComponent<TransformComponent>();
            bool        hasBody   = actor.HasComponent<RigidBodyComponent>();
            BodyType    type
                = hasBody ? actor.GetComponent<RigidBodyComponent>().Type : BodyType::eStatic;

            Body body{};
            body.Entity     = actor;
            PxTransform pose = ToPx(transform);
            if (type == BodyType::eStatic) body.Actor = physics.createRigidStatic(pose);
            else
            {
                const auto& rb = actor.GetComponent<RigidBodyComponent>();
                body.Dynamic   = physics.createRigidDynamic(pose);
                body.Actor     = body.Dynamic;
                body.Kinematic = type == BodyType::eKinematic;

                body.Dynamic->setRigidBodyFlag(PxRigidBodyFlag::eKINEMATIC, body.Kinematic);
                body.Dynamic->setLinearDamping(rb.LinearDamping);
                body.Dynamic->setAngularDamping(rb.AngularDamping);
                body.Dynamic->setActorFlag(PxActorFlag::eDISABLE_GRAVITY, rb.DisableGravity);
                if (rb.LockRotation)
                    body.Dynamic->setRigidDynamicLockFlags(
                        PxRigidDynamicLockFlag::eLOCK_ANGULAR_X
                        | PxRigidDynamicLockFlag::eLOCK_ANGULAR_Y
                        | PxRigidDynamicLockFlag::eLOCK_ANGULAR_Z);
            }
            if (!body.Actor)
            {
                VtCoreErrorTag("Physics", "Can't create a body for '{}'", actor.Name());
                return;
            }

            // Colliders are authored in local space; PhysX shapes aren't
            // scaled by the actor, so scale them here.
            Math::Vec3 scale = glm::abs(transform.Scale);
            if (actor.HasComponent<CapsuleColliderComponent>())
            {
                const auto& capsule = actor.GetComponent<CapsuleColliderComponent>();
                f32 radius = capsule.Radius * glm::max(scale.x, scale.z);
                radius     = radius > 0.001f ? radius : 0.001f;
                f32 half   = capsule.Height * scale.y * 0.5f - radius;
                // PhysX capsules lie along X: stand them up along Y.
                AttachShape(*body.Actor, PxCapsuleGeometry(radius, half > 0.001f ? half : 0.001f),
                            ToPx(capsule.Offset * scale), capsule.Material,
                            PxQuat(PxHalfPi, PxVec3(0.0f, 0.0f, 1.0f)));
            }
            if (actor.HasComponent<MeshColliderComponent>())
            {
                const auto* meshComponent = Owner.Registry().try_get<MeshComponent>(body.Entity);
                PxTriangleMesh* triangles
                    = meshComponent && meshComponent->Mesh ? Cook(*meshComponent->Mesh) : nullptr;
                if (type != BodyType::eStatic)
                    VtCoreWarnTag("Physics", "'{}': mesh colliders are static only",
                                  actor.Name());
                else if (triangles)
                    AttachShape(*body.Actor,
                                PxTriangleMeshGeometry(triangles, PxMeshScale(ToPx(scale))),
                                PxVec3(0.0f),
                                actor.GetComponent<MeshColliderComponent>().Material);
            }
            if (actor.HasComponent<BoxColliderComponent>())
            {
                const auto& box = actor.GetComponent<BoxColliderComponent>();
                Math::Vec3  half = glm::max(box.HalfExtents * scale, Math::Vec3(0.001f));
                AttachShape(*body.Actor, PxBoxGeometry(ToPx(half)),
                            ToPx(box.Offset * scale), box.Material);
            }
            if (actor.HasComponent<SphereColliderComponent>())
            {
                const auto& sphere = actor.GetComponent<SphereColliderComponent>();
                f32 radius = sphere.Radius * glm::max(scale.x, glm::max(scale.y, scale.z));
                AttachShape(*body.Actor, PxSphereGeometry(radius > 0.001f ? radius : 0.001f),
                            ToPx(sphere.Offset * scale), sphere.Material);
            }

            if (body.Dynamic)
            {
                const auto& rb   = actor.GetComponent<RigidBodyComponent>();
                f32         mass = rb.Mass > 0.0f ? rb.Mass : 1.0f;
                PxRigidBodyExt::setMassAndUpdateInertia(*body.Dynamic, mass);
            }

            body.Actor->userData = EntityToUserData(body.Entity);
            World->addActor(*body.Actor);
            Bodies.PushBack(body);
        }

        PxTriangleMesh* Cook(const Mesh& mesh)
        {
            for (const auto& cooked : CookedMeshes)
                if (cooked.Source == &mesh) return cooked.Triangles;

            const auto& vertices = mesh.Vertices();
            const auto& indices  = mesh.Indices();
            if (vertices.Empty() || indices.Size() < 3) return nullptr;

            PxTriangleMeshDesc desc;
            desc.points.count     = static_cast<PxU32>(vertices.Size());
            desc.points.stride    = sizeof(Mesh::Vertex);
            desc.points.data      = &vertices[0].Position;
            desc.triangles.count  = static_cast<PxU32>(indices.Size() / 3);
            desc.triangles.stride = 3 * sizeof(u32);
            desc.triangles.data   = indices.Raw();

            auto&           physics = PhysicsEngine::Internal::Physics();
            PxCookingParams params(physics.getTolerancesScale());
            PxTriangleMesh* triangles
                = PxCreateTriangleMesh(params, desc, physics.getPhysicsInsertionCallback());
            if (!triangles)
                VtCoreErrorTag("Physics", "Can't cook a collision mesh for '{}'",
                               mesh.FilePath());
            else
                VtCoreInfoTag("Physics", "Cooked '{}': {} triangles", mesh.FilePath(),
                              desc.triangles.count);
            CookedMeshes.PushBack({&mesh, triangles});
            return triangles;
        }

        void CreateBodies()
        {
            auto& registry = Owner.Registry();
            for (auto entity : registry.view<TransformComponent>())
            {
                if (!registry.any_of<RigidBodyComponent, BoxColliderComponent,
                                     SphereColliderComponent, CapsuleColliderComponent,
                                     MeshColliderComponent>(entity))
                    continue;
                CreateBody(Actor(entity, Owner));
            }
        }

        PxRigidActor* FindActor(entt::entity entity)
        {
            for (auto& body : Bodies)
                if (body.Entity == entity) return body.Actor;
            return nullptr;
        }

        /// Ball joints with twist/swing limits (JointComponent), made once
        /// every body exists.
        void CreateJoints()
        {
            auto& physics  = PhysicsEngine::Internal::Physics();
            auto& registry = Owner.Registry();
            for (auto entity : registry.view<JointComponent>())
            {
                const auto&   joint  = registry.get<JointComponent>(entity);
                PxRigidActor* first  = FindActor(entity);
                PxRigidActor* second = registry.valid(joint.Connected)
                                         ? FindActor(joint.Connected)
                                         : nullptr;
                if (!first || !second) continue;

                PxTransform world(ToPx(joint.Anchor), RotationFromX(ToPx(joint.Axis)));
                PxTransform frame0 = first->getGlobalPose().getInverse() * world;
                PxTransform frame1 = second->getGlobalPose().getInverse() * world;
                PxD6Joint*  d6 = PxD6JointCreate(physics, first, frame0, second, frame1);
                if (!d6) continue;

                f32 twist = PxClamp(joint.TwistLimit, 1.0f, 179.0f) * PxPi / 180.0f;
                f32 swing = PxClamp(joint.SwingLimit, 1.0f, 179.0f) * PxPi / 180.0f;
                d6->setMotion(PxD6Axis::eTWIST, PxD6Motion::eLIMITED);
                d6->setMotion(PxD6Axis::eSWING1, PxD6Motion::eLIMITED);
                d6->setMotion(PxD6Axis::eSWING2, PxD6Motion::eLIMITED);
                d6->setTwistLimit(PxJointAngularLimitPair(-twist, twist));
                d6->setSwingLimit(PxJointLimitCone(swing, swing));
                Joints.PushBack(d6);

                // Chains of jointed bodies need more solver iterations to
                // hold together when hit.
                for (PxRigidActor* actor : {first, second})
                    if (auto* dynamic = actor->is<PxRigidDynamic>())
                        dynamic->setSolverIterationCounts(16, 4);
            }
        }

        /// The body's RigidBodyComponent, if its actor still has one.
        RigidBodyComponent* RigidBody(const Body& body)
        {
            auto& registry = Owner.Registry();
            if (!registry.valid(body.Entity)) return nullptr;
            return registry.try_get<RigidBodyComponent>(body.Entity);
        }
        TransformComponent* Transform(const Body& body)
        {
            auto& registry = Owner.Registry();
            if (!registry.valid(body.Entity)) return nullptr;
            return registry.try_get<TransformComponent>(body.Entity);
        }

        /// Gameplay's velocity changes and impulses, once per Simulate.
        void ApplyCommands()
        {
            for (auto& body : Bodies)
            {
                if (!body.Dynamic || body.Kinematic) continue;
                auto* rb = RigidBody(body);
                if (!rb) continue;
                if (rb->VelocityDirty)
                {
                    body.Dynamic->setLinearVelocity(ToPx(rb->LinearVelocity));
                    rb->VelocityDirty = false;
                }
                if (rb->PendingImpulse != Math::Vec3(0.0f))
                {
                    body.Dynamic->addForce(ToPx(rb->PendingImpulse), PxForceMode::eIMPULSE);
                    rb->PendingImpulse = Math::Vec3(0.0f);
                }
                for (const auto& point : rb->PendingPointImpulses)
                    PxRigidBodyExt::addForceAtPos(*body.Dynamic, ToPx(point.Impulse),
                                                  ToPx(point.Position), PxForceMode::eIMPULSE);
                rb->PendingPointImpulses.Clear();
            }
        }

        void Step(f32 step)
        {
            for (auto& body : Bodies)
            {
                if (!body.Dynamic) continue;
                if (body.Kinematic)
                {
                    if (auto* transform = Transform(body))
                        body.Dynamic->setKinematicTarget(ToPx(*transform));
                    continue;
                }
                // PhysX clears forces after every simulate: re-apply the
                // frame's force on each substep.
                auto* rb = RigidBody(body);
                if (rb && rb->PendingForce != Math::Vec3(0.0f))
                    body.Dynamic->addForce(ToPx(rb->PendingForce), PxForceMode::eFORCE);
            }
            World->simulate(step);
            World->fetchResults(true);
        }

        void WriteBack()
        {
            for (auto& body : Bodies)
            {
                if (!body.Dynamic || body.Kinematic) continue;
                auto* transform = Transform(body);
                if (!transform) continue;

                PxTransform pose       = body.Dynamic->getGlobalPose();
                transform->Translation = FromPx(pose.p);
                transform->Rotation    = FromPx(pose.q);
                if (auto* rb = RigidBody(body))
                {
                    rb->LinearVelocity = FromPx(body.Dynamic->getLinearVelocity());
                    rb->PendingForce   = Math::Vec3(0.0f);
                }
            }
        }

        void Release()
        {
            for (auto* joint : Joints) joint->release();
            Joints.Clear();
            for (auto& body : Bodies)
                if (body.Actor) body.Actor->release();
            Bodies.Clear();
            if (World) World->release();
            World = nullptr;
            for (auto& cooked : CookedMeshes)
                if (cooked.Triangles) cooked.Triangles->release();
            CookedMeshes.Clear();
        }
    };

    PhysicsScene::PhysicsScene(Scene& scene)
        : m_Impl(new Impl(scene))
    {
        VtProfileFunction();
        if (!PhysicsEngine::IsInitialized())
        {
            VtCoreErrorTag("Physics", "PhysicsScene created before PhysicsEngine::Initialize");
            return;
        }
        m_Impl->CreateWorld();
        if (!m_Impl->World)
        {
            VtCoreErrorTag("Physics", "Can't create the PhysX scene");
            return;
        }
        m_Impl->CreateBodies();
        m_Impl->CreateJoints();
        VtCoreInfoTag("Physics", "Physics scene for '{}': {} bodies", scene.Name(),
                      m_Impl->Bodies.Size());
    }

    PhysicsScene::~PhysicsScene()
    {
        m_Impl->Release();
        delete m_Impl;
    }

    void PhysicsScene::Simulate(f32 deltaTime)
    {
        VtProfileFunction();
        if (!m_Impl->World) return;

        const auto& settings = PhysicsEngine::Settings();
        f32         step     = settings.FixedTimestep > 0.0f ? settings.FixedTimestep
                                                             : 1.0f / 60.0f;
        u32         maxSteps = settings.MaxSubsteps ? settings.MaxSubsteps : 1;

        m_Impl->Accumulator += deltaTime;
        if (m_Impl->Accumulator > step * maxSteps) m_Impl->Accumulator = step * maxSteps;

        m_Impl->ApplyCommands();
        // Catching up costs more frames than it saves once a step takes longer
        // than the time it simulates: then the slow frame makes the next one
        // slower still. Stop when the steps have used up one step's worth of
        // real time and let the world run slow for a moment instead.
        Timer budget;
        u32   steps = 0;
        while (m_Impl->Accumulator >= step && steps < maxSteps)
        {
            m_Impl->Step(step);
            m_Impl->Accumulator -= step;
            ++steps;
            if (budget.ElapsedTime().Seconds() > step)
            {
                m_Impl->Accumulator = 0.0f;
                break;
            }
        }
        if (steps > 0) m_Impl->WriteBack();
    }

    bool PhysicsScene::Raycast(const f32 origin[3], const f32 direction[3],
                               f32 maxDistance, RaycastHit& outHit,
                               u32 ignoredEntity) const
    {
        if (!m_Impl->World) return false;

        PxVec3 dir(direction[0], direction[1], direction[2]);
        if (dir.magnitudeSquared() < 1e-12f || !(maxDistance > 0.0f)) return false;
        dir.normalize();

        PxRaycastBuffer    buffer;
        PxQueryFilterData  filterData;
        filterData.flags = PxQueryFlag::eSTATIC | PxQueryFlag::eDYNAMIC
                         | PxQueryFlag::ePREFILTER;
        IgnoreActorFilter filter(ignoredEntity);

        bool hit = m_Impl->World->raycast(PxVec3(origin[0], origin[1], origin[2]), dir,
                                          maxDistance, buffer, PxHitFlag::eDEFAULT,
                                          filterData, &filter);
        if (!hit || !buffer.hasBlock) return false;

        const auto& block = buffer.block;
        outHit.Entity     = block.actor ? UserDataToEntity(block.actor->userData) : ~0u;
        outHit.Distance   = block.distance;
        outHit.Position[0] = block.position.x;
        outHit.Position[1] = block.position.y;
        outHit.Position[2] = block.position.z;
        outHit.Normal[0]   = block.normal.x;
        outHit.Normal[1]   = block.normal.y;
        outHit.Normal[2]   = block.normal.z;
        return true;
    }

    usize PhysicsScene::BodyCount() const { return m_Impl->Bodies.Size(); }
} // namespace Vortex
