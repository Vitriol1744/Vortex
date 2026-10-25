/*
 * Created by v1tr10l7 on 15.09.2024.
 * Copyright (c) 2024-2024, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include <Prism/Memory/Ref.hpp>
#include <Prism/String/String.hpp>
#include <Vortex/Core/Timestep.hpp>
#include <imgui.h>

namespace Vortex
{
    class Scene;
    namespace Editor
    {
        class ProjectChangedEvent;
        class Panel : public RefCounted<Panel>
        {
          public:
            Panel() = default;
            explicit inline Panel(StringView name)
                : m_Name(name)
            {
            }
            virtual ~Panel() = default;

            inline void Draw()
            {
                if (!Enabled) return;
                ImGui::Begin(m_Name.Raw(), &Enabled);
                OnImGuiRender(Enabled);
                ImGui::End();
            }

            inline StringView Name() const { return m_Name; }

            bool              Enabled = true;

          protected:
            String       m_Name = "Unnamed";

            virtual void OnImGuiRender(bool& isOpen) {}
            virtual void OnUpdate(Timestep ts) {}

            virtual void SetSceneContext(const ::Ref<Scene>& context) {}
            virtual void OnProjectChanged(const ProjectChangedEvent& event) {}

            friend class PanelManager;
        };
    }; // namespace Editor
}; // namespace Vortex
