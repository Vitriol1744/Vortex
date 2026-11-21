/*
 * Created by v1tr10l7 on 18.11.2026.
 * Copyright (c) 2024-2026, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include <Vortex/Core/Layer.hpp>

#include <chrono>
#include <mutex>
#include <thread>

import Prism.Memory.Scope;
import Horizon.Installs;
import Horizon.Manifest;
import Horizon.Platform;
import Horizon.Settings;
import Horizon.Templates;

namespace Horizon
{
    /// The whole launcher UI: projects, engine installs and settings.
    class LauncherLayer final : public Vortex::Layer
    {
      public:
        LauncherLayer();
        ~LauncherLayer() override;

        void OnAttach() override;
        void OnUpdate(Vortex::Timestep) override;
        void OnImGuiRender() override;

      private:
        enum class Page
        {
            eProjects,
            eEngines,
            eSettings,
        };
        enum class FetchState
        {
            eIdle,
            eLoading,
            eReady,
            eFailed,
        };
        struct Toast
        {
            String Text;
            bool   Error = false;
            double Shown = 0.0;
        };
        using Clock = std::chrono::steady_clock;

        Settings              m_Settings;
        Vector<EngineInstall> m_Installs;
        Page                  m_Page = Page::eProjects;

        // The manifest is fetched on a worker thread (std::thread and
        // std::mutex: Prism has no threads, and its Mutex spins).
        std::mutex            m_FetchMutex;
        std::thread           m_FetchThread;
        FetchState            m_FetchState = FetchState::eIdle;
        Optional<Manifest>    m_Manifest;
        String                m_FetchError;

        Scope<InstallJob>     m_Job;
        // Download speed, smoothed.
        u64                   m_SpeedBytes = 0;
        Clock::time_point     m_SpeedTime;
        double                m_BytesPerSecond = 0.0;

        // Projects page.
        char                  m_Search[128]{};
        bool                  m_OpenNewProject = false;
        char                  m_NewName[128]{};
        char                  m_NewLocation[1024]{};
        String                m_NewEngine;
        /// The chosen engine's project templates, and which engine they're for.
        Vector<ProjectTemplate> m_Templates;
        String                m_TemplatesEngine;
        String                m_NewTemplate;

        // Settings page: edited here, applied by Save.
        Settings              m_Draft;
        char                  m_DraftManifestUrl[1024]{};
        char                  m_DraftInstallDir[1024]{};
        char                  m_DraftProjectsDir[1024]{};
        char                  m_DraftExtraArgs[512]{};

        // An install waiting for "Remove engine?" to be confirmed.
        String                m_PendingRemoval;

        Vector<Toast>         m_Toasts;

        void                  DrawSidebar();
        void                  DrawNavItem(const char* icon, const char* label, Page page,
                                          bool badge);
        void                  DrawSidebarFooter();
        /// Title, subtitle, and room on the right for `actionsWidth` pixels
        /// of buttons; returns with the cursor placed for them.
        void                  DrawPageHeader(const char* title, StringView subtitle,
                                             float actionsWidth);

        void                  DrawProjectsPage();
        void                  DrawNewProjectTile(float width, float height);
        void                  DrawProjectTile(usize index, float width, float height);
        void                  DrawNewProjectModal();
        void                  DrawTemplatePicker();
        void                  RefreshTemplates();

        void                  DrawEnginesPage();
        void                  DrawUpdateBanner(const Release& release);
        void                  DrawInstalledEngines();
        void                  DrawAvailableReleases();
        void                  DrawReleaseCard(const Release& release);
        void                  DrawInstallProgress(float width);
        void                  DrawRemoveEngineModal();

        void                  DrawSettingsPage();
        void                  DrawToasts();
        bool                  DrawEngineCombo(const char* id, String& engineId);

        void                  HandleShortcuts();
        void                  Rescan();
        void                  FetchManifest();
        void                  StartInstall(const Release& release);
        void                  LoadDraft();
        bool                  DraftChanged() const;
        void                  ApplyDraft();
        void                  SaveSettings();
        void                  BrowseForProject();
        void                  OpenProject(const Path& path, StringView engineId);
        void                  CreateProject();
        void                  AfterLaunch();
        void                  Notify(String message, bool isError = false);

        const EngineInstall*  FindInstall(StringView id) const;
        bool                  IsInstalled(StringView version) const;
        /// A stable release newer than every install, if the manifest has one.
        const Release*        AvailableUpdate() const;
    };
} // namespace Horizon
