/*
 * Created by v1tr10l7 on 13.09.2026.
 * Copyright (c) 2024-2026, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include <Vortex/Core/Events/Event.hpp>
#include <Vortex/Core/Time.hpp>
#include <Vortex/Project/Project.hpp>

namespace Vortex::Editor
{
    struct EditorSessionState
    {
        Path         ActiveScenePath;
        Vector<Path> RecentScenes;
        usize        MaxRecentScenes = 10;
    };
    class EditorProject : public RefCounted<EditorProject>
    {
      public:
        EditorProject() = default;
        explicit EditorProject(::Ref<Project> runtimeProject,
                               PathView       projectConfigPath);

        inline PathView       ConfigPath() const { return m_ProjectConfigPath; }
        inline PathView       Path() const { return m_ProjectPath; }
        inline ::Ref<Project> RuntimeProject() const
        {
            return m_RuntimeProject;
        }

        bool                  Flush();

        void                  SetActiveScene(PathView relativePath);
        inline const PathView ActiveScene() const
        {
            return m_SessionState.ActiveScenePath;
        }
        inline const Vector<::Path>& RecentScenes() const
        {
            return m_SessionState.RecentScenes;
        }

        inline bool                        IsDirty() const { return m_IsDirty; }
        inline void                        MarkDirty() { m_IsDirty = true; }

        inline static ::Ref<EditorProject> Active() { return s_Active; }
        static void                 SwitchTo(::Ref<EditorProject> project);

        static ::Ref<EditorProject> New(const ProjectConfig& config,
                                        PathView             projectPath,
                                        PathView templatePath = ""_pv);
        static ::Ref<EditorProject> Load(PathView projectPath);

      private:
        ::Ref<Project>                     m_RuntimeProject;
        EditorSessionState                 m_SessionState;
        ::Path                             m_ProjectPath       = "";
        ::Path                             m_ProjectConfigPath = "";
        bool                               m_IsDirty           = false;

        inline static ::Ref<EditorProject> s_Active            = nullptr;

        friend class ::Ref<EditorProject>;
        template <typename T, typename... Args>
        friend constexpr ::Ref<T> Prism::CreateRef(Args&&... args);
    };

    struct ProjectChangedEvent
    {
        static constexpr EventCategory Category = EventCategory::eEditor;
        ::Ref<EditorProject>           Project;
        time_t                         Time;
    };
}; // namespace Vortex::Editor
