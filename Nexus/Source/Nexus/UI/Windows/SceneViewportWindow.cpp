/*
 * Created by v1tr10l7 on 17.11.2026.
 * Copyright (c) 2024-2026, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#include <Nexus/UI/Windows/SceneViewportWindow.hpp>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/matrix_decompose.hpp>

#include <ImGuizmo.h>

#include <Vortex/Core/Math/Ray.hpp>
#include <Vortex/Core/Profiler.hpp>
#include <Vortex/Renderer/ImGui/ImGui.hpp>
#include <Vortex/Scene/Components.hpp>
#include <Vortex/Scene/FirstPerson.hpp>
#include <Vortex/Scene/SceneRenderer.hpp>

#include <cfloat>
#include <limits>

using namespace Vortex::Math;

namespace Vortex::Editor
{
    namespace
    {
        AABB TransformAABB(const AABB& aabb, const Mat4& transform)
        {
            AABB result;
            result.Min = Vec3(FLT_MAX);
            result.Max = Vec3(-FLT_MAX);
            for (u32 corner = 0; corner < 8; ++corner)
            {
                Vec3 point((corner & 1) ? aabb.Max.x : aabb.Min.x,
                           (corner & 2) ? aabb.Max.y : aabb.Min.y,
                           (corner & 4) ? aabb.Max.z : aabb.Min.z);
                Vec3 transformed = Vec3(transform * Vec4(point, 1.0f));
                result.Min       = glm::min(result.Min, transformed);
                result.Max       = glm::max(result.Max, transformed);
            }
            return result;
        }

        /// A ray through `pixel` (relative to the image) in world space.
        Ray CastRay(ImVec2 pixel, ImVec2 size, Camera& camera)
        {
            // Vulkan NDC: y grows downwards, like pixels.
            f32  x       = 2.0f * pixel.x / size.x - 1.0f;
            f32  y       = 2.0f * pixel.y / size.y - 1.0f;
            Mat4 inverse = glm::inverse(camera.ViewProjection());
            Vec4 start   = inverse * Vec4(x, y, 0.0f, 1.0f);
            Vec4 end     = inverse * Vec4(x, y, 1.0f, 1.0f);
            start /= start.w;
            end /= end.w;
            return {Vec3(start), glm::normalize(Vec3(end) - Vec3(start))};
        }

        /// The camera Play mode renders with: the scene's first-person
        /// character's, if it has one.
        Camera* GameCamera(EditorContext& context, f32* fieldOfView = nullptr)
        {
            if (context.Scenes.State() != SceneState::ePlay) return nullptr;
            return FirstPerson::ActiveCamera(context.ActiveScene(), fieldOfView);
        }

        entt::entity PickEntity(const Ray& ray, Scene& scene)
        {
            entt::entity closest  = entt::null;
            f32          distance = std::numeric_limits<f32>::max();
            for (auto entity : scene.Registry().view<TransformComponent>())
            {
                Actor actor(entity, scene);
                AABB  local(Vec3(-0.5f), Vec3(0.5f));
                if (actor.HasComponent<MeshComponent>()
                    && actor.GetComponent<MeshComponent>().Mesh)
                    local = actor.GetComponent<MeshComponent>().Mesh->AABB();

                AABB world = TransformAABB(
                    local, Mat4(actor.GetComponent<TransformComponent>()));
                f32  hit   = 0.0f;
                if (ray.IntersectsAABB(world, hit) && hit < distance)
                {
                    distance = hit;
                    closest  = entity;
                }
            }
            return closest;
        }
    } // namespace

    void SceneViewportWindow::OnAttach(EditorContext&)
    {
        m_Target = CreateRef<SceneRenderTarget>(RHI::Extent2D{1280, 720},
                                                "Editor viewport");
        m_Size   = m_Target->Size();
    }

    void SceneViewportWindow::OnUpdate(EditorContext& context, float)
    {
        // The camera flies in edit and simulate mode (play mode belongs to
        // the game).
        if (Open && m_Focused && context.Scenes.State() != SceneState::ePlay)
            context.Camera.Update(context.Preferences.Camera);
    }

    void SceneViewportWindow::OnRender(EditorContext& context)
    {
        auto size = m_Target->Size();
        if (size.Width != m_Size.Width || size.Height != m_Size.Height)
            m_Target->Resize(m_Size);
        SceneRenderer::ViewOptions options{};
        f32 fieldOfView = 75.0f;
        if (Camera* game = GameCamera(context, &fieldOfView))
        {
            f32 aspect = static_cast<f32>(m_Size.Width)
                       / static_cast<f32>(m_Size.Height ? m_Size.Height : 1);
            // A near plane close enough for the gun in hand.
            game->SetPerspective(glm::radians(fieldOfView), aspect, 0.03f, 500.0f);
            options.DrawGrid = false;
            SceneRenderer::AddToGraph(Renderer::Graph(), context.ActiveScene(), *game,
                                      *m_Target, options);
            return;
        }
        UpdateProjection(context);
        options.DrawGrid = context.Viewport.ShowGrid;
        SceneRenderer::AddToGraph(Renderer::Graph(), context.ActiveScene(),
                                  context.Camera, *m_Target, options);
    }

    void SceneViewportWindow::UpdateProjection(EditorContext& context)
    {
        const auto& settings = context.Viewport;
        f32         aspect   = static_cast<f32>(m_Size.Width)
                     / static_cast<f32>(m_Size.Height ? m_Size.Height : 1);
        if (aspect == m_AppliedAspect && settings.FieldOfView == m_Applied.FieldOfView
            && settings.Near == m_Applied.Near && settings.Far == m_Applied.Far)
            return;
        context.Camera.SetPerspective(glm::radians(settings.FieldOfView), aspect,
                                      settings.Near, settings.Far);
        m_Applied       = settings;
        m_AppliedAspect = aspect;
    }

    void SceneViewportWindow::OnBeforeBegin(EditorContext&)
    {
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    }
    void SceneViewportWindow::OnAfterBegin(EditorContext&) { ImGui::PopStyleVar(); }

    void SceneViewportWindow::OnDraw(EditorContext& context)
    {
        VtProfileFunction();
        m_Focused      = ImGui::IsWindowFocused();
        m_Hovered      = ImGui::IsWindowHovered();

        ImVec2 region  = ImGui::GetContentRegionAvail();
        m_Size.Width   = static_cast<u32>(region.x > 1.0f ? region.x : 1.0f);
        m_Size.Height  = static_cast<u32>(region.y > 1.0f ? region.y : 1.0f);

        // The target catches up with the size in OnRender; until then the
        // image is stretched over the region.
        m_ImageMin  = ImGui::GetCursorScreenPos();
        m_ImageSize = ImVec2(static_cast<f32>(m_Size.Width),
                             static_cast<f32>(m_Size.Height));
        UI::Image(m_Target->Color(), m_ImageSize);
        bool imageHovered = ImGui::IsItemHovered();

        // Playing a first-person scene: the viewport is the game screen.
        FirstPerson::SetCaptureAllowed(imageHovered);
        if (GameCamera(context))
        {
            DrawGameHud();
            return;
        }

        DrawGizmo(context);
        DrawOverlay(context);

        if (imageHovered && ImGui::IsMouseClicked(ImGuiMouseButton_Left)
            && !ImGuizmo::IsOver() && !ImGuizmo::IsUsing())
        {
            ImVec2 mouse = ImGui::GetMousePos();
            Pick(context, ImVec2(mouse.x - m_ImageMin.x, mouse.y - m_ImageMin.y));
        }
    }

    void SceneViewportWindow::DrawGameHud()
    {
        ImDrawList* draw   = ImGui::GetWindowDrawList();
        ImVec2      center = {m_ImageMin.x + m_ImageSize.x * 0.5f,
                              m_ImageMin.y + m_ImageSize.y * 0.5f};
        bool        captured = FirstPerson::IsMouseCaptured();

        // Crosshair: four ticks around a gap, with a dark outline.
        constexpr f32 kGap = 5.0f, kLength = 9.0f;
        const ImVec2  directions[] = {{1, 0}, {-1, 0}, {0, 1}, {0, -1}};
        for (int pass = 0; pass < 2; ++pass)
            for (const auto& d : directions)
            {
                ImVec2 from{center.x + d.x * kGap, center.y + d.y * kGap};
                ImVec2 to{center.x + d.x * (kGap + kLength), center.y + d.y * (kGap + kLength)};
                draw->AddLine(from, to, pass == 0 ? IM_COL32(0, 0, 0, 160) : IM_COL32(255, 255, 255, 230),
                              pass == 0 ? 4.0f : 2.0f);
            }
        draw->AddCircleFilled(center, 1.5f, IM_COL32(255, 255, 255, 230));

        const char* hint
            = captured ? "WASD move   Space jump   Shift sprint   LMB shoot   Esc release mouse"
                       : "Click the viewport to play";
        ImVec2 size = ImGui::CalcTextSize(hint);
        ImVec2 at{center.x - size.x * 0.5f, m_ImageMin.y + m_ImageSize.y - size.y - 18.0f};
        draw->AddRectFilled({at.x - 10, at.y - 6}, {at.x + size.x + 10, at.y + size.y + 6},
                            IM_COL32(0, 0, 0, 140), 6.0f);
        draw->AddText(at, captured ? IM_COL32(220, 220, 225, 255) : IM_COL32(255, 210, 90, 255),
                      hint);
    }

    void SceneViewportWindow::DrawOverlay(EditorContext& context)
    {
        constexpr struct
        {
            const char* Label;
            GizmoMode   Mode;
        } kButtons[] = {{"Move", GizmoMode::eTranslate},
                        {"Rotate", GizmoMode::eRotate},
                        {"Scale", GizmoMode::eScale}};

        ImGui::SetCursorScreenPos(ImVec2(m_ImageMin.x + 8.0f, m_ImageMin.y + 8.0f));
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 4.0f);
        for (const auto& button : kButtons)
        {
            bool active = Gizmo == button.Mode;
            if (active)
                ImGui::PushStyleColor(ImGuiCol_Button,
                                      ImGui::GetStyleColorVec4(ImGuiCol_ButtonActive));
            if (ImGui::Button(button.Label)) Gizmo = button.Mode;
            if (active) ImGui::PopStyleColor();
            ImGui::SameLine();
        }
        ImGui::Dummy(ImVec2(8.0f, 0.0f));
        ImGui::SameLine();
        ImGui::Checkbox("Grid", &context.Viewport.ShowGrid);
        ImGui::PopStyleVar();
    }

    void SceneViewportWindow::DrawGizmo(EditorContext& context)
    {
        Actor selection = context.Selection();
        if (!selection || !selection.HasComponent<TransformComponent>()) return;

        ImGuizmo::SetOrthographic(false);
        ImGuizmo::SetDrawlist();
        ImGuizmo::SetRect(m_ImageMin.x, m_ImageMin.y, m_ImageSize.x, m_ImageSize.y);

        // ImGuizmo expects OpenGL clip space (Y up): undo the camera's
        // Vulkan flip.
        Mat4 view       = context.Camera.View();
        Mat4 projection = context.Camera.Projection();
        projection[1][1] *= -1.0f;

        auto& component = selection.GetComponent<TransformComponent>();
        Mat4  transform = component;

        ImGuizmo::OPERATION operation = ImGuizmo::TRANSLATE;
        if (Gizmo == GizmoMode::eRotate) operation = ImGuizmo::ROTATE;
        else if (Gizmo == GizmoMode::eScale) operation = ImGuizmo::SCALE;

        // Ctrl snaps: 0.5 units, 15 degrees, 0.1 scale.
        bool  snap      = ImGui::GetIO().KeyCtrl;
        f32   snapValue = Gizmo == GizmoMode::eRotate ? 15.0f
                        : Gizmo == GizmoMode::eScale  ? 0.1f
                                                      : 0.5f;
        f32   snapValues[3] = {snapValue, snapValue, snapValue};

        ImGuizmo::Manipulate(glm::value_ptr(view), glm::value_ptr(projection),
                             operation, ImGuizmo::LOCAL, glm::value_ptr(transform),
                             nullptr, snap ? snapValues : nullptr);
        if (!ImGuizmo::IsUsing()) return;

        Vec3 translation, scale, skew;
        Vec4 perspective;
        Quat rotation;
        if (!glm::decompose(transform, scale, rotation, translation, skew, perspective))
            return;
        component.Translation = translation;
        component.Rotation    = rotation;
        component.Scale       = scale;
    }

    void SceneViewportWindow::Pick(EditorContext& context, ImVec2 mouse)
    {
        if (mouse.x < 0.0f || mouse.y < 0.0f || mouse.x >= m_ImageSize.x
            || mouse.y >= m_ImageSize.y)
            return;
        Ray ray = CastRay(mouse, m_ImageSize, context.Camera);
        context.Select(PickEntity(ray, context.ActiveScene()));
    }
} // namespace Vortex::Editor
