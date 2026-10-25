/*
 * Created by v1tr10l7 on 13.09.2026.
 * Copyright (c) 2024-2026, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include <Nexus/UI/Panel.hpp>

#include <Prism/Containers/UnorderedMap.hpp>
#include <Prism/Core/Concepts.hpp>
#include <Prism/Memory/Ref.hpp>
#include <Prism/Utility/Delegate.hpp>

namespace Vortex
{
    class Scene;
    class Project;
    class Event;

    namespace Editor
    {
        enum class PanelCategory
        {
            eEdit,
            eView,
            eTools
        };

        struct PanelDescriptor
        {
            String             ID;
            String             Name;
            PanelCategory      Category = PanelCategory::eView;
            ::Ref<class Panel> Panel    = nullptr;
            bool               IsOpen   = true;
        };

        class PanelManager
        {
          public:
            PanelManager()                               = default;
            ~PanelManager()                              = default;

            PanelManager(const PanelManager&)            = delete;
            PanelManager& operator=(const PanelManager&) = delete;

            void          OnImGuiRender()
            {
                for (auto& [id, data] : m_Panels)
                {
                    if (data.IsOpen && data.Panel) data.Panel->Draw();
                    // data.Panel->OnImGuiRender(data.IsOpen);
                }
            }

            void SetSceneContext(const ::Ref<Scene>& context)
            {
                for (auto& [id, data] : m_Panels)
                {
                    if (data.Panel) data.Panel->SetSceneContext(context);
                }
            }

            void OnProjectChanged(const ProjectChangedEvent& e)
            {
                for (auto& [id, data] : m_Panels)
                {
                    if (data.Panel) data.Panel->OnProjectChanged(e);
                }
            }

            // --- Panel Registration ---
            template <typename TPanel, typename... TArgs>
                requires(DerivedFrom<TPanel, Panel>)::Ref<TPanel>
            AddPanel(PanelCategory category, StringView id,
                     StringView displayName, bool isOpenByDefault,
                     TArgs&&... args)
            {
                String idStr(id);
                if (m_Panels.Contains(idStr)) return nullptr;

                auto panelInstance = CreateRef<TPanel>(Forward<TArgs>(args)...);

                m_Panels[idStr]    = PanelDescriptor{.ID   = idStr,
                                                     .Name = String(displayName),
                                                     .Category = category,
                                                     .Panel    = panelInstance,
                                                     .IsOpen   = isOpenByDefault};

                return panelInstance;
            }

            template <typename TPanel, typename... TArgs>
                requires(DerivedFrom<TPanel, Panel>)::Ref<TPanel>
            AddPanel(PanelCategory category, StringView id,
                     bool isOpenByDefault, TArgs&&... args)
            {
                return AddPanel<TPanel>(category, id, id, isOpenByDefault,
                                        Forward<TArgs>(args)...);
            }

            // --- Lookups & Queries ---
            template <typename TPanel>
                requires(DerivedFrom<TPanel, Panel>)::Ref<TPanel>
            PanelAt(StringView id)
            {
                auto it = m_Panels.Find(String(id));
                if (it != m_Panels.end()) return it->Value.Panel.As<TPanel>();
                return nullptr;
            }

            PanelDescriptor* PanelDescriptorAt(StringView id)
            {
                auto it = m_Panels.Find(String(id));
                return (it != m_Panels.end()) ? &it->Value : nullptr;
            }

            void RemovePanel(StringView id) { m_Panels.Erase(String(id)); }

            void
            ForEachInCategory(PanelCategory category,
                              const std::function<void(PanelDescriptor&)>& func)
            {
                for (auto& [id, data] : m_Panels)
                {
                    if (data.Category == category) func(data);
                }
            }

          private:
            UnorderedMap<String, PanelDescriptor> m_Panels;
        };
    }; // namespace Editor
}; // namespace Vortex
