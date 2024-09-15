/*
 * Created by v1tr10l7 on 07.09.2024.
 * Copyright (c) 2024-2024, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include <Vortex/Core/Layer.hpp>
#include <Vortex/Renderer/API/Framebuffer.hpp>
#include <Vortex/Renderer/API/GraphicsPipeline.hpp>
#include <Vortex/Renderer/API/Shader.hpp>
#include <Vortex/Renderer/API/Texture2D.hpp>
#include <Vortex/Scene/Entity.hpp>
#include <Vortex/Scene/Scene.hpp>

namespace Vortex
{
    class EditorLayer : public Layer
    {
      public:
        EditorLayer()
            : Layer("Editor Layer")
        {
        }

        virtual void OnAttach() override;
        virtual void OnDetach() override;

        virtual void OnUpdate() override;
        virtual void OnRender() override;
        virtual void OnImGuiRender() override;

      private:
        bool                  m_ShouldResizeFramebuffer = false;
        Ref<Shader>           m_Shader                  = nullptr;
        Ref<GraphicsPipeline> m_Pipeline                = nullptr;
        Ref<Texture2D>        m_Texture                 = nullptr;
        Ref<Framebuffer>      m_Framebuffer             = nullptr;
        void*                 m_DescriptorSet           = nullptr;
        std::vector<void*>    m_FramebufferDescriptorSets;
        Vec2u                 m_ViewportSize{100, 100};
        Scene                 m_Scene;
        bool                  m_ViewportFocused    = false;
        bool                  m_ViewportHovered    = false;
        EntityID              m_SelectedEntity     = entt::null;
        bool                  m_ShowSceneHierarchy = true;

        void                  DrawStatisticsPanel();
        void                  DrawViewport();
        void                  DrawSceneHierarchyPanel();
        void                  DrawComponents(Entity& entity);
    };
}; // namespace Vortex
