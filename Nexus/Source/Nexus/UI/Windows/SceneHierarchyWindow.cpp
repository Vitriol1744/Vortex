/*
 * Created by v1tr10l7 on 15.09.2024.
 * Copyright (c) 2024-2024, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#include <Nexus/UI/Windows/SceneHierarchyWindow.hpp>
#include <Vortex/Scene/Components.hpp>
#include <Vortex/Window/Window.hpp>
#include <Nexus/EditorContext.hpp>

#include <imgui_internal.h>

import Nexus.Inspector;

namespace Vortex::Editor
{
    namespace
    {
        bool DrawVec3Control(StringView label, Math::Vec3& values,
                             f32 resetValue = 0.0f, f32 columnWidth = 100.0f)
        {
            ImGui::PushID(label.Raw());

            ImGui::Columns(2);
            ImGui::SetColumnWidth(0, columnWidth);
            ImGui::Text("%s", label.Raw());
            ImGui::NextColumn();

            ImGui::PushMultiItemsWidths(3, ImGui::CalcItemWidth());
            ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));

            bool modified   = false;

            f32  lineHeight = GImGui->Font->LegacySize
                           + GImGui->Style.FramePadding.y * 2.0f;
            ImVec2 buttonSize = {lineHeight + 3.0f, lineHeight};

            ImGui::PushStyleColor(ImGuiCol_Button,
                                  ImVec4(0.49f, 0.01f, 0.05f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered,
                                  ImVec4(0.9f, 0.2f, 0.2f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive,
                                  ImVec4(0.8f, 0.1f, 0.15f, 1.0f));

            if (ImGui::Button("X", buttonSize)) values.x = resetValue;
            ImGui::PopStyleColor(3);
            ImGui::SameLine();
            modified |= ImGui::DragFloat("##X", &values.x, 0.1f);
            ImGui::PopItemWidth();
            ImGui::SameLine();

            ImGui::PushStyleColor(ImGuiCol_Button,
                                  ImVec4(0.003f, 0.32f, 0.04f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered,
                                  ImVec4(0.3f, 0.8f, 0.3f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive,
                                  ImVec4(0.2f, 0.7f, 0.2f, 1.0f));
            if (ImGui::Button("Y", buttonSize)) values.y = resetValue;
            ImGui::PopStyleColor(3);
            ImGui::SameLine();
            modified |= ImGui::DragFloat("##Y", &values.y, 0.1f);
            ImGui::PopItemWidth();
            ImGui::SameLine();

            ImGui::PushStyleColor(ImGuiCol_Button,
                                  ImVec4(0.01f, 0.23f, 0.5f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered,
                                  ImVec4(0.2f, 0.35f, 0.9f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive,
                                  ImVec4(0.1f, 0.25f, 0.8f, 1.0f));
            if (ImGui::Button("Z", buttonSize)) values.z = resetValue;

            ImGui::PopStyleColor(3);
            ImGui::SameLine();
            modified |= ImGui::DragFloat("##Z", &values.z, 0.1f);

            ImGui::PopItemWidth();
            ImGui::PopStyleVar();

            ImGui::Columns(1);
            ImGui::PopID();
            return modified;
        }

        template <typename T>
        void DrawComponent(Actor& entity, EditorContext& context)
        {
            static_assert(false);
        }

        template <>
        void DrawComponent<TagComponent>(Actor& entity, EditorContext& context)
        {
            auto& tagComponent = entity.GetComponent<TagComponent>();

            char  buffer[256];
            Memory::Fill(buffer, 0, 256);
            Memory::Copy(buffer, tagComponent.Name.Raw(),
                         tagComponent.Name.Size());

            if (ImGui::InputText("Tag", buffer, 255)) tagComponent = buffer;
        }

        template <>
        void DrawComponent<TransformComponent>(Actor& entity, EditorContext& context)
        {
            auto& transformComponent
                = entity.GetComponent<TransformComponent>();

            DrawVec3Control("Translation", transformComponent.Translation);
            DrawVec3Control("Scale", transformComponent.Scale);
            Math::Vec3 rotation
                = glm::degrees(glm::eulerAngles(transformComponent.Rotation));
            if (DrawVec3Control("Rotation", rotation))
                transformComponent.Rotation = glm::radians(rotation);
        }

        template <>
        void DrawComponent<CameraComponent>(Actor& entity, EditorContext& context)
        {
            auto&       cc = entity.GetComponent<CameraComponent>();

            const char* projectionTypes[] = {"Orthographic", "Perspective"};
            const char* currentProjection
                = projectionTypes[ToUnderlying(cc.Camera.ProjectionType())];

            if (ImGui::BeginCombo("Projection", currentProjection))
            {
                for (u32 type = 0; type < 2; type++)
                {
                    bool isSelected
                        = currentProjection == projectionTypes[type];
                    if (ImGui::Selectable(projectionTypes[type], isSelected))
                    {
                        currentProjection = projectionTypes[type];
                        auto t            = static_cast<ProjectionType>(type);
                        if (t == ProjectionType::eOrthographic)
                            ; // cc.Camera.SetOrthographic();
                        else if (t == ProjectionType::ePerspective)
                            ; // cc.Camera.SetPerspective();
                    }

                    if (isSelected) ImGui::SetItemDefaultFocus();
                }
                ImGui::EndCombo();
            }
        }

        template <>
        void DrawComponent<SpriteComponent>(Actor& entity, EditorContext& context)
        {
            auto& spriteComponent = entity.GetComponent<SpriteComponent>();
            ImGui::ColorEdit4("Color", glm::value_ptr(spriteComponent.Color));
        }
        template <>
        void DrawComponent<MeshComponent>(Actor& entity, EditorContext& context)
        {
            Inspector::MeshComponentPanel(entity, entity.GetComponent<MeshComponent>(),
                                          context);
        }

        template <>
        void DrawComponent<AnimationComponent>(Actor& entity, EditorContext& context)
        {
            auto& animation = entity.GetComponent<AnimationComponent>();
            const Vector<AnimationClip>* clips = nullptr;
            if (entity.HasComponent<MeshComponent>()
                && entity.GetComponent<MeshComponent>().Mesh)
                clips = &entity.GetComponent<MeshComponent>().Mesh->Animations();

            if (!clips || clips->Empty())
            {
                ImGui::TextDisabled("The mesh has no animations");
                return;
            }
            if (animation.Clip >= clips->Size()) animation.Clip = 0;

            auto clipName = [&](u32 index)
            {
                const auto& name = (*clips)[index].Name;
                return name.Empty() ? String("Unnamed clip") : name;
            };
            if (ImGui::BeginCombo("Clip", clipName(animation.Clip).Raw()))
            {
                for (u32 index = 0; index < clips->Size(); ++index)
                {
                    bool selected = index == animation.Clip;
                    ImGui::PushID(static_cast<int>(index));
                    if (ImGui::Selectable(clipName(index).Raw(), selected))
                    {
                        animation.Clip = index;
                        animation.Time = 0.0f;
                    }
                    if (selected) ImGui::SetItemDefaultFocus();
                    ImGui::PopID();
                }
                ImGui::EndCombo();
            }
            ImGui::Checkbox("Playing", &animation.Playing);
            ImGui::DragFloat("Speed", &animation.Speed, 0.01f, -4.0f, 4.0f);
            f32 duration = (*clips)[animation.Clip].DurationSeconds();
            ImGui::SliderFloat("Time", &animation.Time, 0.0f,
                               duration > 0.0f ? duration : 1.0f, "%.2f s");
        }

        void DrawColliderMaterial(ColliderMaterial& material)
        {
            ImGui::DragFloat("Friction", &material.Friction, 0.01f, 0.0f, 2.0f);
            ImGui::DragFloat("Restitution", &material.Restitution, 0.01f, 0.0f, 1.0f);
        }

        template <>
        void DrawComponent<RigidBodyComponent>(Actor& entity, EditorContext& context)
        {
            using BodyType = RigidBodyComponent::BodyType;
            auto&                 rb      = entity.GetComponent<RigidBodyComponent>();
            constexpr const char* kTypes[] = {"Static", "Dynamic", "Kinematic"};
            int                   type    = static_cast<int>(rb.Type);
            if (ImGui::Combo("Type", &type, kTypes, IM_ARRAYSIZE(kTypes)))
                rb.Type = static_cast<BodyType>(type);

            if (rb.Type != BodyType::eDynamic) return;
            ImGui::DragFloat("Mass", &rb.Mass, 0.05f, 0.001f, 10000.0f);
            ImGui::DragFloat("Linear Damping", &rb.LinearDamping, 0.01f, 0.0f, 100.0f);
            ImGui::DragFloat("Angular Damping", &rb.AngularDamping, 0.01f, 0.0f, 100.0f);
            ImGui::Checkbox("Disable Gravity", &rb.DisableGravity);
            ImGui::Checkbox("Lock Rotation", &rb.LockRotation);
        }

        template <>
        void DrawComponent<BoxColliderComponent>(Actor& entity, EditorContext& context)
        {
            auto& box = entity.GetComponent<BoxColliderComponent>();
            DrawVec3Control("Half Extents", box.HalfExtents, 0.5f);
            DrawVec3Control("Offset", box.Offset);
            DrawColliderMaterial(box.Material);
        }

        template <>
        void DrawComponent<SphereColliderComponent>(Actor& entity, EditorContext& context)
        {
            auto& sphere = entity.GetComponent<SphereColliderComponent>();
            ImGui::DragFloat("Radius", &sphere.Radius, 0.01f, 0.001f, 1000.0f);
            DrawVec3Control("Offset", sphere.Offset);
            DrawColliderMaterial(sphere.Material);
        }

        template <>
        void DrawComponent<ScriptComponent>(Actor& entity, EditorContext& context)
        {
            Inspector::ScriptComponentPanel(entity, entity.GetComponent<ScriptComponent>(),
                                            context);
        }

        template <typename T>
        void DrawComponentNode(Actor& entity, StringView name, EditorContext& context)
        {
            if (!entity.HasComponent<T>()) return;

            ImGuiTreeNodeFlags nodeFlags = ImGuiTreeNodeFlags_DefaultOpen
                                         | ImGuiTreeNodeFlags_AllowOverlap;

            ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(4, 4));
            bool opened = ImGui::TreeNodeEx(
                reinterpret_cast<void*>(typeid(T).hash_code()), nodeFlags, "%s",
                name.Raw());

            ImGui::SameLine(ImGui::GetWindowWidth() - 25.0f);
            bool removeComponent = ImGui::Button("-", ImVec2(20, 20));
            ImGui::PopStyleVar();

            if (opened)
            {
                DrawComponent<T>(entity, context);
                ImGui::TreePop();
            }

            if (removeComponent) entity.RemoveComponent<T>();
        }
    }; // namespace

    void SceneHierarchyWindow::OnDraw(EditorContext& context)
    {
        Scene&               scene    = context.ActiveScene();
        entt::entity         selected = context.Selection();
        Vector<entt::entity> removed;

        // Only the visible rows are drawn: a played scene can have hundreds
        // of actors (bullet holes, debris).
        Vector<entt::entity> entities;
        for (const auto entityID : scene.Registry().view<entt::entity>())
            entities.PushBack(entityID);

        ImGuiListClipper clipper;
        clipper.Begin(static_cast<int>(entities.Size()));
        while (clipper.Step())
        {
            for (int row = clipper.DisplayStart; row < clipper.DisplayEnd; ++row)
            {
                const auto  entityID = entities[static_cast<usize>(row)];
                Actor       entity(entityID, scene);
                const char* tag = "Untagged";
                if (entity.HasComponent<TagComponent>())
                    tag = entity.GetComponent<TagComponent>();

                ImGui::PushID(static_cast<int>(entt::to_integral(entityID)));
                ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow
                                         | ImGuiTreeNodeFlags_SpanAvailWidth
                                         | ImGuiTreeNodeFlags_AllowOverlap
                                         | ImGuiTreeNodeFlags_Leaf;
                if (selected == entityID) flags |= ImGuiTreeNodeFlags_Selected;

                ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(4, 4));
                bool opened = ImGui::TreeNodeEx("##actor", flags, "%s", tag);
                if (ImGui::IsItemClicked()) context.Select(entityID);

                ImGui::SameLine(ImGui::GetWindowWidth() - 25.0f);
                bool remove = ImGui::Button("-", ImVec2(20, 20));
                ImGui::PopStyleVar();

                if (opened) ImGui::TreePop();
                if (remove) removed.PushBack(entityID);
                ImGui::PopID();
            }
        }
        for (const auto entityID : removed)
        {
            if (entityID == context.Selection()) context.ClearSelection();
            scene.RemoveActor(Actor(entityID, scene));
        }

        // Clicking empty space deselects.
        if (ImGui::IsMouseClicked(ImGuiMouseButton_Left)
            && ImGui::IsWindowHovered() && !ImGui::IsAnyItemHovered())
            context.ClearSelection();

        if (ImGui::BeginPopupContextWindow(nullptr,
                                           ImGuiPopupFlags_MouseButtonRight
                                               | ImGuiPopupFlags_NoOpenOverItems))
        {
            if (ImGui::MenuItem("Add Actor"))
            {
                Actor actor = scene.AddActor();
                actor.AddComponent<TagComponent>("Actor");
                actor.AddComponent<TransformComponent>();
                context.Select(actor);
            }
            ImGui::EndPopup();
        }
    }

    void InspectorWindow::OnDraw(EditorContext& context)
    {
        Actor entity = context.Selection();
        if (!entity)
        {
            ImGui::TextDisabled("Select an actor to inspect it");
            return;
        }

        DrawComponentNode<TagComponent>(entity, "Tag", context);
        DrawComponentNode<TransformComponent>(entity, "Transform", context);
        DrawComponentNode<CameraComponent>(entity, "Camera", context);
        DrawComponentNode<SpriteComponent>(entity, "Sprite", context);
        DrawComponentNode<MeshComponent>(entity, "Mesh", context);
        DrawComponentNode<AnimationComponent>(entity, "Animation", context);
        DrawComponentNode<ScriptComponent>(entity, "Script", context);
        DrawComponentNode<RigidBodyComponent>(entity, "Rigid Body", context);
        DrawComponentNode<BoxColliderComponent>(entity, "Box Collider", context);
        DrawComponentNode<SphereColliderComponent>(entity, "Sphere Collider", context);

        ImGui::Separator();
        if (ImGui::Button("Add Component", ImVec2(-1.0f, 0.0f)))
            ImGui::OpenPopup("AddComponent");
        if (ImGui::BeginPopup("AddComponent"))
        {
            auto item = [&]<typename T>(const char* name)
            {
                if (entity.HasComponent<T>() || !ImGui::MenuItem(name)) return;
                entity.AddComponent<T>();
                ImGui::CloseCurrentPopup();
            };
            item.operator()<TagComponent>("Tag");
            item.operator()<TransformComponent>("Transform");
            item.operator()<SpriteComponent>("Sprite");
            item.operator()<MeshComponent>("Mesh");
            item.operator()<CameraComponent>("Camera");
            item.operator()<AnimationComponent>("Animation");
            item.operator()<ScriptComponent>("Script");
            item.operator()<RigidBodyComponent>("Rigid Body");
            item.operator()<BoxColliderComponent>("Box Collider");
            item.operator()<SphereColliderComponent>("Sphere Collider");
            ImGui::EndPopup();
        }
    }
}; // namespace Vortex::Editor
