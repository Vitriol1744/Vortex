/*
 * Created by v1tr10l7 on 07.09.2024.
 * Copyright (c) 2024-2024, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/quaternion.hpp>

#include <Vortex/Core/Layer.hpp>
#include <Vortex/Core/Math/Ray.hpp>
#include <Vortex/Core/Process.hpp>
#include <Vortex/Engine/Application.hpp>
#include <Vortex/Window/Window.hpp>

#include <Vortex/Project/Project.hpp>

#include <Vortex/Renderer/ImGui/ModalManager.hpp>
#include <Vortex/Renderer/Mesh.hpp>
#include <Vortex/Renderer/RHI/Framebuffer.hpp>
#include <Vortex/Renderer/RHI/GraphicsPipeline.hpp>
#include <Vortex/Renderer/RHI/IndexBuffer.hpp>
#include <Vortex/Renderer/RHI/Material.hpp>
#include <Vortex/Renderer/RHI/Shader.hpp>
#include <Vortex/Renderer/RHI/UniformBuffer.hpp>
#include <Vortex/Renderer/RHI/VertexBuffer.hpp>
#include <Vortex/Renderer/TextureAsset.hpp>
#include <Vortex/Scene/Actor.hpp>
#include <Vortex/Scene/Scene.hpp>

#include <Nexus/EditorCamera.hpp>
#include <Nexus/EditorPreferences.hpp>
#include <Nexus/EditorProject.hpp>
#include <Nexus/UI/Modals/NewProjectModal.hpp>
#include <Nexus/UI/Overlays/SettingsOverlay.hpp>
#include <Nexus/UI/PanelManager.hpp>
#include <Nexus/UI/Panels/ConsolePanel.hpp>
#include <Nexus/UI/Panels/ContentBrowserPanel.hpp>
#include <Nexus/UI/Panels/SceneHierarchyPanel.hpp>
#include <Nexus/UI/Panels/ViewportPanel.hpp>
#include <Prism/Containers/KeyValuePair.hpp>

#include <ImGuizmo.h>

namespace Vortex::Editor
{
    enum class SceneState
    {
        eEdit,
        ePlay,
        eSimulate,
    };
    struct UniformBufferObject
    {
        alignas(16) Mat4 Model;
        alignas(16) Mat4 ViewProjection;
    };

    struct Vertex
    {
        Math::Vec3 Pos;
        Math::Vec3 Color;
        Math::Vec2 TexCoords;
    };

    class EditorLayer : public Layer
    {
      public:
        EditorLayer()
            : Layer("Editor Layer")
            , m_App(Application::Get())
        {
        }

        virtual void OnAttach() override;
        virtual void OnDetach() override;

        virtual void OnUpdate(Timestep deltaTime) override;
        virtual void OnRender() override;
        virtual void OnImGuiRender() override;

      private:
        Application*               m_App                      = nullptr;
        bool                       m_ShouldResizeFramebuffer  = false;
        ::Ref<Shader>              m_Shader                   = nullptr;
        ::Ref<Framebuffer>         m_Framebuffer              = nullptr;

        void*                      m_FramebufferDescriptorSet = nullptr;
        Math::Vec2u                m_ViewportSize{100, 100};
        Path                       m_CurrentScenePath = "";
        Path                       m_PendingScenePath = "";
        ::Ref<EditorProject>       m_PendingProject   = nullptr;
        bool                       m_ViewportFocused  = false;
        bool                       m_ViewportHovered  = false;
        Array<glm::vec2, 2>        m_ViewportBounds;

        PanelManager               m_PanelManager;
        ::Ref<SceneHierarchyPanel> m_SceneHierarchyPanel = nullptr;
        ::Ref<ConsolePanel>        m_ConsolePanel        = nullptr;
        SettingsOverlay            m_SettingsOverlay;
        EditorPreferences          m_Preferences;
        bool                       m_ShowSettingsModal = false;
        bool                       m_ThemeEditor       = false;
        enum class ScriptCompilationState
        {
            eIdle,
            eCompiling,
            eSuccess,
            eFailed
        };

        EditorCamera           m_Camera;
        SceneState             m_SceneState   = SceneState::eEdit;
        Scope<Process>         m_BuildTask    = nullptr;
        ScriptCompilationState m_CompileState = ScriptCompilationState::eIdle;
        String                 m_CompileOutput;
        isize                  m_CompileExitCode = 0;

        ::Ref<Scene>           m_EditorScene     = nullptr;
        ::Ref<Scene>           m_RuntimeScene    = nullptr;

        ::Ref<ViewportPanel>   m_EditorViewport  = nullptr;
        ::Ref<ViewportPanel>   m_RuntimeViewport = nullptr;

        ::Ref<TextureAsset>    m_IconPlay;
        ::Ref<TextureAsset>    m_IconPause;
        ::Ref<TextureAsset>    m_IconStep;
        ::Ref<TextureAsset>    m_IconStop;
        ::Ref<TextureAsset>    m_IconSimulate;

        Math::Ray CastRay(f32 mouseX, f32 mouseY, f32 viewportWidth,
                          f32 viewportHeight, Camera& camera);

        Actor     PickActor(const Math::Ray& ray, const Scene& scene);

        bool      OpenStartupProject();
        void      BuildAssembly();

        void      OnScenePlay();
        void      OnSceneStop();
        void      OnSceneSimulate();
        void      OnSceneStep();

        template <typename TPanel, typename... Args>
            requires(DerivedFrom<TPanel, Panel>)
        inline ::Ref<TPanel> AddPanel(PanelCategory category, StringView id,
                                      bool isOpenByDefault, Args&&... args)
        {
            return m_PanelManager.AddPanel<TPanel>(
                category, id, isOpenByDefault, Forward<Args>(args)...);
        }

        void UpdateDescriptorSets();
        void DrawMenuBar();
        void DrawToolbar();
        void DrawStatisticsPanel();

        void DrawSceneViewport();
        void DrawGameViewport();

        void OnProjectChanged(const ProjectChangedEvent& e);
        void OnMouseButtonPressed(const MouseButtonPressedEvent& e);
    };
}; // namespace Vortex::Editor
