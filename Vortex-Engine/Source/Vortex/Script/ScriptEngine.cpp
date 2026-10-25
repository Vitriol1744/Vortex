/*
 * Created by v1tr10l7 on 24.05.2026.
 * Copyright (c) 2024-2026, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#include <Prism/Containers/UnorderedMap.hpp>
#include <Prism/String/StringUtils.hpp>
#include <Prism/Utility/PathView.hpp>
#include <Vortex/Core/Core.hpp>
#include <Vortex/Scene/Actor.hpp>
#include <Vortex/Scene/Components.hpp>
#include <Vortex/Script/Glue/ScriptGlue.hpp>
#include <Vortex/Script/ScriptEngine.hpp>
#include <Vortex/Script/ScriptInstance.hpp>
#include <Vortex/Window/Input/Keyboard.hpp>

#include <filesystem>
#include <string_view>

#include <Coral/Array.hpp>
#include <Coral/Attribute.hpp>
#include <Coral/GC.hpp>
#include <Coral/HostInstance.hpp>
#include <Coral/String.hpp>
#include <Coral/TypeCache.hpp>

#ifdef VT_PLATFORM_DIST
    #define VtICallValidateParam(param)
    #define VtICallValidateParamV(param, value)
#else
    #define VtICallValidateParam(param)                                        \
        {                                                                      \
            if (!(param))                                                      \
            {                                                                  \
                VtCoreError(                                                   \
                    "{} called with an invalid value ({}) for parameter '{}'", \
                    PM_FUNCTION_NAME, param, #param);                          \
            }                                                                  \
        }
    #define VtICallValidateParamV(param, value)                                \
        {                                                                      \
            if (!(param))                                                      \
            {                                                                  \
                VtCoreError(                                                   \
                    "{} called with an invalid value ({}) for parameter "      \
                    "'{}'.",                                                   \
                    PM_FUNCTION_NAME, value, #param);                          \
            }                                                                  \
        }
#endif

namespace Vortex
{
    using stdstr_view = ::std::string_view;
    namespace stdfs   = ::std::filesystem;
    namespace ScriptEngine
    {
        namespace
        {
            bool                       s_Initialized = false;
            Coral::HostInstance        s_HostInstance;
            Coral::AssemblyLoadContext s_LoadContext;

            inline constexpr PathView  s_CoreAssemblyPath
                = "DotNet/Vortex-ScriptCore.dll";
            constexpr PathView      s_AppAssemblyPath = "DotNet/Sandbox.dll";

            Coral::ManagedAssembly* s_CoreAssembly    = nullptr;
            Coral::ManagedAssembly* s_AppAssembly     = nullptr;
            UnorderedMap<u64, ::Ref<ScriptInstance>> s_EntityInstances;

            UnorderedMap<Coral::TypeId, std::function<void(Actor&)>>
                s_CreateComponentFuncs;
            UnorderedMap<Coral::TypeId, std::function<bool(Actor&)>>
                s_HasComponentFuncs;
            UnorderedMap<Coral::TypeId, std::function<void(Actor&)>>
                 s_RemoveComponentFuncs;

            void MessageCallback(stdstr_view message, Coral::MessageLevel level)
            {
                switch (level)
                {
                    case Coral::MessageLevel::Trace:
                        VtCoreTrace("Coral: {}", message);
                        break;
                    case Coral::MessageLevel::Info:
                        VtCoreInfo("Coral: {}", message);
                        break;
                    case Coral::MessageLevel::Warning:
                        VtCoreWarn("Coral: {}", message);
                        break;
                    case Coral::MessageLevel::Error:
                        VtCoreError("Coral: {}", message);
                        break;

                    default:
                        VtCoreError("Coral: Unknown log level, payload => {}",
                                    message);
                }
            }

            void ExceptionCallback(stdstr_view inMessage)
            {
                VtCoreError("ScriptEngine: Unhandled native exception => {}",
                            inMessage);
            }

            inline static Actor GetActor(Entity entity)
            {
                auto scene = Scene::Active();
                VtCoreAssertMsg(scene, "Active scene is null");

                return Actor(entity, *scene);
            }
            struct Transform
            {
                glm::vec3 Translation = glm::vec3(0.0f);
                glm::vec3 Rotation    = glm::vec3(0.0f);
                glm::vec3 Scale       = glm::vec3(1.0f);
            };

            void Actor_CreateComponent(Entity                entityID,
                                       Coral::ReflectionType componentType)
            {
                auto actor = GetActor(entityID);
                // VtICallValidateParamV(actor, entityID);

                if (!actor) return;

                Coral::Type& type = componentType;

                if (!type) return;

                // Coral::ScopedString typeName = type.GetFullName();

                if (auto it = s_HasComponentFuncs.Find(type.GetTypeId());
                    it != s_HasComponentFuncs.end() && it->Value(actor))
                {
                    // WarnWithTrace("Attempting to add duplicate component '{}'
                    // to entity '{}', ignoring.", std::string(typeName),
                    // entity.Name());
                    return;
                }

                if (auto it = s_CreateComponentFuncs.Find(type.GetTypeId());
                    it != s_CreateComponentFuncs.end())
                    return;
            }

            bool Actor_HasComponent(Entity                entityID,
                                    Coral::ReflectionType componentType)
            {
                auto entity = GetActor(entityID);

                if (!entity) return false;

                Coral::Type& type = componentType;

                if (!type) return false;

                // Coral::ScopedString typeName = type.GetFullName();

                if (auto it = s_HasComponentFuncs.Find(type.GetTypeId());
                    it != s_HasComponentFuncs.end())
                {
                    auto func = it->Value;
                    return func(entity);
                }

                // ErrorWithTrace("Cannot check if entity '{}' has a component
                // of type '{}'. That component hasn't been registered with the
                // engine.", entity.Name(), std::string(typeName));
                return false;
            }

            bool Actor_RemoveComponent(Entity                entityID,
                                       Coral::ReflectionType componentType)
            {
                auto entity = GetActor(entityID);

                if (!entity) return false;

                Coral::Type& type = componentType;

                if (!type) return false;

                // Coral::ScopedString typeName = type.GetFullName();

                if (auto it = s_HasComponentFuncs.Find(type.GetTypeId());
                    it == s_HasComponentFuncs.end() || !it->Value(entity))
                {
                    // WarnWithTrace("Tried to remove component '{}' from entity
                    // '{}' even though it doesn't have that component.",
                    // std::string(typeName), entity.Name());
                    return false;
                }

                if (auto it = s_RemoveComponentFuncs.Find(type.GetTypeId());
                    it != s_RemoveComponentFuncs.end())
                {
                    it->Value(entity);
                    return true;
                }

                // ErrorWithTrace("Cannot remove component of type '{}' from
                // entity '{}'. That component hasn't been registered with the
                // engine.", std::string(typeName), entity.Name());
                return false;
            }

            Coral::String TagComponent_GetTag(Entity entityID)
            {
                auto        entity       = GetActor(entityID);
                // VtICallValidateParamV(entityID, entityID);

                const auto& tagComponent = entity.GetComponent<TagComponent>();
                return Coral::String::New(tagComponent.Name.Raw());
            }

            void TagComponent_SetTag(Entity entityID, Coral::String inTag)
            {
                auto  entity       = GetActor(entityID);
                // VtICallValidateParamV(entity, entityID);
                auto& tagComponent = entity.GetComponent<TagComponent>();
                tagComponent.Name  = inTag.Data();
            }

            void TransformComponent_SetTranslation(Entity    entityID,
                                                   glm::vec3 inTranslation);

            void TransformComponent_SetRotation(Entity     entityID,
                                                glm::vec3* inRotation);

            void TransformComponent_GetTransform(Entity     entityID,
                                                 Transform* outTransform)
            {
                auto        actor = GetActor(entityID);

                const auto& tc    = actor.GetComponent<TransformComponent>();
                outTransform->Translation = tc.Translation;
                outTransform->Rotation    = tc.RotationEuler();
                outTransform->Scale       = tc.Scale;
            }

            void TransformComponent_SetTransform(Entity     entityID,
                                                 Transform* inTransform)
            {
                auto actor = GetActor(entityID);

                if (!inTransform)
                {
                    VtCoreError(
                        "Attempting to set a null transform for entity '{}'",
                        actor.Name());
                    return;
                }

                auto& tc = actor.GetComponent<TransformComponent>();
                // call TransformComponent_Set... methods so that
                // SetTransform(transform) behaves the same way as setting
                // individual components.
                TransformComponent_SetTranslation(entityID,
                                                  inTransform->Translation);
                TransformComponent_SetRotation(entityID,
                                               &inTransform->Rotation);
                tc.Scale = inTransform->Scale;
            }

            void TransformComponent_GetTranslation(Entity     entityID,
                                                   glm::vec3* outTranslation)
            {
                auto actor = GetActor(entityID);
                *outTranslation
                    = actor.GetComponent<TransformComponent>().Translation;
            }

            void TransformComponent_SetTranslation(Entity    entityID,
                                                   glm::vec3 inTranslation)
            {
                auto actor = GetActor(entityID);

                actor.GetComponent<TransformComponent>().Translation
                    = inTranslation;
            }

            void TransformComponent_GetRotation(Entity     entityID,
                                                glm::vec3* outRotation)
            {
                auto actor = GetActor(entityID);
                *outRotation
                    = actor.GetComponent<TransformComponent>().RotationEuler();
            }

            void TransformComponent_SetRotation(Entity     entityID,
                                                glm::vec3* inRotation)
            {
                auto actor = GetActor(entityID);

                if (!inRotation)
                {
                    VtCoreError(
                        "Attempting to set null rotation for entity '{}'!",
                        actor.Name());
                    return;
                }

                actor.GetComponent<TransformComponent>().SetRotationEuler(
                    *inRotation);
            }

            void TransformComponent_GetRotationQuat(Entity     entityID,
                                                    glm::quat* outRotation)
            {
                auto actor = GetActor(entityID);
                *outRotation
                    = actor.GetComponent<TransformComponent>().Rotation;
            }

            void TransformComponent_SetRotationQuat(Entity     entityID,
                                                    glm::quat* inRotation)
            {
                auto actor = GetActor(entityID);

                if (!inRotation)
                {
                    VtCoreError(
                        "Attempting to set null rotation for entity '{}'!",
                        actor.Name());
                    return;
                }

                actor.GetComponent<TransformComponent>().Rotation = *inRotation;
            }

            void TransformComponent_GetScale(Entity     entityID,
                                             glm::vec3* outScale)
            {
                auto actor = GetActor(entityID);
                *outScale  = actor.GetComponent<TransformComponent>().Scale;
            }

            void TransformComponent_SetScale(Entity     entityID,
                                             glm::vec3* inScale)
            {
                auto actor = GetActor(entityID);

                if (!actor) return;

                if (!inScale)
                {
                    VtCoreError("Attempting to set null scale for entity '{}'!",
                                actor.Name());
                    return;
                }

                actor.GetComponent<TransformComponent>().Scale = *inScale;
            }

        } // namespace
        void VT_SCRIPT_API Initialize()
        {
            VtCoreAssertMsg(!s_Initialized, "ScriptEngine already initialized");

            VtCoreTrace("ScriptEngine: Initializing...");
            Coral::HostSettings settings;
            // TODO(v1tr10l7): Configurable path?
            settings.CoralDirectory
                = (stdfs::current_path() / "DotNet").string();
            settings.MessageCallback   = MessageCallback;
            settings.ExceptionCallback = ExceptionCallback;
            auto status                = s_HostInstance.Initialize(settings);

            if (status != Coral::CoralInitStatus::Success)
            {
                VtCoreError("Failed to initialize coral, error => {}",
                            StringUtils::ToString(status));
                return;
            }

            s_LoadContext
                = s_HostInstance.CreateAssemblyLoadContext("VortexLoadContext");

            constexpr StringView coreAssemblyPath
                = "DotNet/Vortex-ScriptCore.dll";
            constexpr StringView appAssemblyPath = "DotNet/Sandbox.dll";
            s_CoreAssembly
                = &s_LoadContext.LoadAssembly(coreAssemblyPath.Raw());
            s_AppAssembly = &s_LoadContext.LoadAssembly(appAssemblyPath.Raw());
            ScriptGlue::RegisterAll(*s_CoreAssembly);

            s_CoreAssembly->AddInternalCall(
                "Vortex.InternalCalls", "Actor_CreateComponent",
                reinterpret_cast<void*>(&Actor_CreateComponent));
            s_CoreAssembly->AddInternalCall(
                "Vortex.InternalCalls", "Actor_HasComponent",
                reinterpret_cast<void*>(&Actor_HasComponent));
            s_CoreAssembly->AddInternalCall(
                "Vortex.InternalCalls", "Actor_RemoveComponent",
                reinterpret_cast<void*>(&Actor_RemoveComponent));
            s_CoreAssembly->AddInternalCall(
                "Vortex.InternalCalls", "TagComponent_GetTag",
                reinterpret_cast<void*>(&TagComponent_GetTag));
            s_CoreAssembly->AddInternalCall(
                "Vortex.InternalCalls", "TagComponent_SetTag",
                reinterpret_cast<void*>(&TagComponent_SetTag));
            s_CoreAssembly->AddInternalCall(
                "Vortex.InternalCalls", "TransformComponent_GetTransform",
                reinterpret_cast<void*>(&TransformComponent_GetTransform));
            s_CoreAssembly->AddInternalCall(
                "Vortex.InternalCalls", "TransformComponent_SetTransform",
                reinterpret_cast<void*>(&TransformComponent_SetTransform));
            s_CoreAssembly->AddInternalCall(
                "Vortex.InternalCalls", "TransformComponent_GetTranslation",
                reinterpret_cast<void*>(&TransformComponent_GetTranslation));
            s_CoreAssembly->AddInternalCall(
                "Vortex.InternalCalls", "TransformComponent_SetTranslation",
                reinterpret_cast<void*>(&TransformComponent_SetTranslation));
            s_CoreAssembly->AddInternalCall(
                "Vortex.InternalCalls", "TransformComponent_GetRotation",
                reinterpret_cast<void*>(&TransformComponent_GetRotation));
            s_CoreAssembly->AddInternalCall(
                "Vortex.InternalCalls", "TransformComponent_SetRotation",
                reinterpret_cast<void*>(&TransformComponent_SetRotation));
            s_CoreAssembly->AddInternalCall(
                "Vortex.InternalCalls", "TransformComponent_GetScale",
                reinterpret_cast<void*>(&TransformComponent_GetScale));
            s_CoreAssembly->AddInternalCall(
                "Vortex.InternalCalls", "TransformComponent_SetScale",
                reinterpret_cast<void*>(&TransformComponent_SetScale));
            s_CoreAssembly->AddInternalCall(
                "Vortex.InternalCalls", "TransformComponent_GetRotationQuat",
                reinterpret_cast<void*>(&TransformComponent_GetRotationQuat));
            s_CoreAssembly->AddInternalCall(
                "Vortex.InternalCalls", "TransformComponent_SetRotationQuat",
                reinterpret_cast<void*>(&TransformComponent_SetRotationQuat));
            s_CoreAssembly->UploadInternalCalls();

            s_Initialized = true;
        }

        void VT_SCRIPT_API Shutdown()
        {
            if (!s_Initialized) return;
            VtCoreTrace("ScriptEngine: Shutting down...");

            s_HostInstance.UnloadAssemblyLoadContext(s_LoadContext);
            Coral::GC::Collect();
            Coral::GC::WaitForPendingFinalizers();

            s_LoadContext = {};
            s_HostInstance.Shutdown();

            s_Initialized = false;
        }

        bool VT_SCRIPT_API ReloadAssembly()
        {
            VtCoreInfo("Reloading script assembly: {}", s_AppAssemblyPath);

            ClearScriptInstances();
            Shutdown();
            Initialize();

            // TODO(v1tr10l7): Restore Script Instances

            VtCoreInfo("Assembly reloaded successfully.");
            return true;
        }

        void VT_SCRIPT_API AddInternalCall(StringView internalClass,
                                           StringView internalCall, void* func)
        {
            auto& coreAssembly = *s_CoreAssembly;
            coreAssembly.AddInternalCall(internalClass.Raw(),
                                         internalCall.Raw(), func);
        }

        void VT_SCRIPT_API OnCreateEntity(u64 entityID, StringView className)
        {
            auto& appAssembly  = *s_AppAssembly;
            auto& coreAssembly = *s_CoreAssembly;

            auto& scriptType   = appAssembly.GetLocalType(className.Raw());
            auto& actorType    = coreAssembly.GetLocalType("Vortex.Actor");

            // 2. Perform subclass verification
            if (!scriptType.IsSubclassOf(actorType))
            {
                VtCoreError("Class {} does not derive from Vortex.Actor",
                            className);
                return;
            }

            auto instance = CreateRef<ScriptInstance>(scriptType, entityID);
            s_EntityInstances[entityID] = instance;

            instance->InvokeOnCreate();
        }
        void VT_SCRIPT_API OnUpdateEntity(u64 entityID, Timestep ts)
        {
            auto it = s_EntityInstances.Find(entityID);
            if (it != s_EntityInstances.end())
            {
                if (it->Value) it->Value->InvokeOnUpdate(ts);
            }
        }

        void VT_SCRIPT_API OnUpdateRuntime(float deltaTime)
        {
            for (auto& [entityID, instance] : s_EntityInstances)
            {
                if (instance) instance->InvokeOnUpdate(deltaTime);
            }
        }

        void VT_SCRIPT_API OnDestroyEntity(u64 entityID)
        {
            auto it = s_EntityInstances.Find(entityID);
            if (it != s_EntityInstances.end())
            {
                if (it->Value)
                {
                    it->Value->InvokeOnDestroy();
                    it->Value->ManagedObject().Destroy();
                }
                s_EntityInstances.Erase(it);
            }
        }

        void VT_SCRIPT_API ClearScriptInstances()
        {
            for (auto& [id, instance] : s_EntityInstances)
            {
                if (instance)
                {
                    instance->InvokeOnDestroy();
                    instance->ManagedObject().Destroy();
                }
            }
            s_EntityInstances.Clear();
        }
    }; // namespace ScriptEngine
}; // namespace Vortex
