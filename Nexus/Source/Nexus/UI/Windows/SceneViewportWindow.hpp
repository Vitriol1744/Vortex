/*
 * Created by v1tr10l7 on 17.11.2026.
 * Copyright (c) 2024-2026, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include <Nexus/EditorContext.hpp>

#include <imgui.h>

import Vortex.Renderer;
import Vortex.Renderer.RenderTarget;

namespace Vortex::Editor
{
    enum class GizmoMode
    {
        eTranslate,
        eRotate,
        eScale,
    };

    /// The scene as the editor camera sees it: camera control, selection by
    /// clicking, and the transform gizmo of the selected actor.
    class SceneViewportWindow final : public EditorWindow
    {
      public:
        SceneViewportWindow()
            : EditorWindow("Viewport")
        {
            Closable = false;
            Flags    = ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse;
        }

        void        OnAttach(EditorContext& context) override;
        void        OnUpdate(EditorContext& context, float deltaTime) override;
        void        OnRender(EditorContext& context) override;
        void        OnDraw(EditorContext& context) override;
        void        OnBeforeBegin(EditorContext& context) override;
        void        OnAfterBegin(EditorContext& context) override;

        inline bool IsHovered() const { return m_Hovered; }
        inline bool IsFocused() const { return m_Focused; }

        GizmoMode   Gizmo = GizmoMode::eTranslate;

      private:
        ::Ref<SceneRenderTarget> m_Target  = nullptr;
        RHI::Extent2D            m_Size    = {1, 1};
        bool                     m_Focused = false;
        bool                     m_Hovered = false;
        ImVec2                   m_ImageMin{};
        ImVec2                   m_ImageSize{};
        /// Projection last applied to the camera (re-applied on change).
        ViewportSettings         m_Applied{};
        f32                      m_AppliedAspect = 0.0f;

        void                     UpdateProjection(EditorContext& context);
        /// Gizmo mode buttons and view toggles over the image.
        void                     DrawOverlay(EditorContext& context);
        /// Play mode with a first-person camera: crosshair and controls.
        void DrawGameHud();
        void                     DrawGizmo(EditorContext& context);
        void                     Pick(EditorContext& context, ImVec2 mouse);
    };
} // namespace Vortex::Editor
