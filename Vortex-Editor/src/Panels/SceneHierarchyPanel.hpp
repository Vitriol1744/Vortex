/*
 * Created by v1tr10l7 on 15.09.2024.
 * Copyright (c) 2024-2024, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include <Vortex/Core/Log/Log.hpp>

#include <Vortex/Scene/Entity.hpp>
#include <Vortex/Scene/Scene.hpp>

#include "Panels/Panel.hpp"

namespace Vortex
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
            m_Scene          = &scene;
            m_SelectedEntity = entt::null;
        }

        inline Entity GetSelectedEntity() const { return m_SelectedEntity; }

      protected:
        virtual void OnImGuiRender() override;

      private:
        Scene* m_Scene;
        Entity m_SelectedEntity;

        void   DrawContextMenu();
    };
}; // namespace Vortex
