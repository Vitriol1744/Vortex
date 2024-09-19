/*
 * Created by v1tr10l7 on 15.09.2024.
 * Copyright (c) 2024-2024, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#include "Panels/SceneHierarchyPanel.hpp"

#include <imgui_internal.h>

namespace Vortex
{
    namespace
    {
        bool DrawVec3Control(std::string_view label, Vec3& values,
                             f32 resetValue = 0.0f, f32 columnWidth = 100.0f)
        {
            ImGui::PushID(label.data());

            ImGui::Columns(2);
            ImGui::SetColumnWidth(0, columnWidth);
            ImGui::Text("%s", label.data());
            ImGui::NextColumn();

            ImGui::PushMultiItemsWidths(3, ImGui::CalcItemWidth());
            ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));

            bool modified = false;

            f32  lineHeight
                = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
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
        void DrawComponent(Entity& entity)
        {
            static_assert(false);
        }

        template <>
        void DrawComponent<TagComponent>(Entity& entity)
        {
            auto& tagComponent = entity.GetComponent<TagComponent>();

            char  buffer[256];
            std::memset(buffer, 0, 256);
            std::memcpy(buffer, tagComponent.Name.data(),
                        tagComponent.Name.size());

            if (ImGui::InputText("Tag", buffer, 255)) tagComponent = buffer;
        }

        template <>
        void DrawComponent<TransformComponent>(Entity& entity)
        {
            auto& transformComponent
                = entity.GetComponent<TransformComponent>();

            DrawVec3Control("Translation", transformComponent.Translation);
            DrawVec3Control("Scale", transformComponent.Scale);
            Vec3 rotation
                = glm::degrees(glm::eulerAngles(transformComponent.Rotation));
            if (DrawVec3Control("Rotation", rotation))
                transformComponent.Rotation = glm::radians(rotation);
        }

        template <>
        void DrawComponent<CameraComponent>(Entity& entity)
        {
            auto&       cc = entity.GetComponent<CameraComponent>();

            const char* projectionTypes[] = {"Orthographic", "Perspective"};
            const char* currentProjection = projectionTypes[std::to_underlying(
                cc.Camera.GetProjectionType())];

            if (ImGui::BeginCombo("Projection", currentProjection))
            {
                for (u32 i = 0; i < 2; i++)
                {
                    bool isSelected = currentProjection == projectionTypes[i];
                    if (ImGui::Selectable(projectionTypes[i], isSelected))
                    {
                        currentProjection = projectionTypes[i];
                        // cc.Camera.Set
                    }

                    if (isSelected) ImGui::SetItemDefaultFocus();
                }
                ImGui::EndCombo();
            }
        }

        template <>
        void DrawComponent<SpriteComponent>(Entity& entity)
        {
            auto& spriteComponent = entity.GetComponent<SpriteComponent>();
            ImGui::ColorEdit4("Color", glm::value_ptr(spriteComponent.Color));
        }

        template <typename T>
        void DrawComponentNode(Entity& entity, std::string_view name)
        {
            if (!entity.HasComponent<T>()) return;

            ImGuiTreeNodeFlags nodeFlags = ImGuiTreeNodeFlags_DefaultOpen
                                         | ImGuiTreeNodeFlags_AllowItemOverlap;

            ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(4, 4));
            bool opened = ImGui::TreeNodeEx(
                reinterpret_cast<void*>(typeid(T).hash_code()), nodeFlags, "%s",
                name.data());

            ImGui::SameLine(ImGui::GetWindowWidth() - 25.0f);
            bool removeComponent = ImGui::Button("-", ImVec2(20, 20));
            ImGui::PopStyleVar();

            if (opened)
            {
                auto& component = entity.GetComponent<T>();
                DrawComponent<T>(entity);
                ImGui::TreePop();
            }

            if (removeComponent) entity.RemoveComponent<T>();
        }
    }; // namespace

    void SceneHierarchyPanel::OnImGuiRender()
    {
        auto view = m_Scene->GetRegistry().view<entt::entity>();
        for (const auto entityID : view)
        {
            Entity entity(entityID, *m_Scene);

            ImGui::PushID(reinterpret_cast<void*>(entityID));
            const char* entityTag = "Untagged";
            if (entity.HasComponent<TagComponent>())
                entityTag = entity.GetComponent<TagComponent>();

            ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow
                                     | ImGuiTreeNodeFlags_SpanAvailWidth
                                     | ImGuiTreeNodeFlags_AllowItemOverlap;
            if (m_SelectedEntity == entityID)
                flags |= ImGuiTreeNodeFlags_Selected;

            ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(4, 4));
            bool opened
                = ImGui::TreeNodeEx((void*)entityID, flags, "%s", entityTag);

            if (ImGui::IsItemClicked() && m_SelectedEntity != entityID)
                m_SelectedEntity = Entity(entityID, *m_Scene);

            ImGui::SameLine(ImGui::GetWindowWidth() - 25.0f);
            bool removeEntity = ImGui::Button("-", ImVec2(20, 20));
            ImGui::PopStyleVar();

            if (opened) ImGui::TreePop();
            if (removeEntity)
            {
                if (entityID == m_SelectedEntity) m_SelectedEntity = entt::null;
                m_Scene->RemoveEntity(entity);
            }
            ImGui::PopID();
        }

        if (ImGui::IsMouseDown(ImGuiMouseButton_Left)
            && ImGui::IsWindowHovered())
            m_SelectedEntity = entt::null;

        if (ImGui::BeginPopupContextWindow(nullptr,
                                           ImGuiPopupFlags_MouseButtonRight))
        {
            if (ImGui::MenuItem("Add entity"))
            {
                Entity newEntity = m_Scene->AddEntity();
                auto&  tc        = newEntity.AddComponent<TagComponent>();
                tc.Name          = "Unnamed";
            }
            ImGui::EndPopup();
        }

        ImGui::Begin("Properties");
        Entity entity(m_SelectedEntity, *m_Scene);

        if (entity)
        {
            DrawContextMenu();
            DrawComponentNode<TagComponent>(entity, "Tag");
            DrawComponentNode<TransformComponent>(entity, "Transform");
            DrawComponentNode<SpriteComponent>(entity, "Sprite");
        }
        ImGui::End();
    }

    void SceneHierarchyPanel::DrawContextMenu()
    {
        const char* tag = "Untagged";
        if (m_SelectedEntity.HasComponent<TagComponent>())
            tag = m_SelectedEntity.GetComponent<TagComponent>();

        if (ImGui::BeginPopupContextWindow(0, ImGuiPopupFlags_MouseButtonRight))
        {
            if (ImGui::MenuItem("Add Component"))
            {
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }
    }
}; // namespace Vortex
