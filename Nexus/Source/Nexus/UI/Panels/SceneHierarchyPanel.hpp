/*
 * Created by v1tr10l7 on 15.09.2024.
 * Copyright (c) 2024-2024, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include <Vortex/Core/Log/Log.hpp>

#include <Vortex/Scene/Actor.hpp>
#include <Vortex/Scene/Scene.hpp>

#include <Nexus/UI/Panel.hpp>

namespace Vortex::Editor
{
    class SceneHierarchyPanel final : public Panel
    {
      public:
        SceneHierarchyPanel(Scene& scene)
            : Panel("Scene Hierarchy")
            , m_Scene(&scene)
        {
        }

        void SetScene(Scene& scene)
        {
            m_Scene         = &scene;
            m_SelectedActor = entt::null;
        }

        inline void  SetSelectedActor(Actor actor) { m_SelectedActor = actor; }
        inline Actor SelectedActor() const { return m_SelectedActor; }

      protected:
        virtual void OnImGuiRender(bool& isOpen) override;

      private:
        Scene* m_Scene;
        Actor  m_SelectedActor;

        void   DrawContextMenu();
    };
}; // namespace Vortex::Editor
