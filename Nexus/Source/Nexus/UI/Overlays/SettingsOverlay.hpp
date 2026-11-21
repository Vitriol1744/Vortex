/*
 * Created by v1tr10l7 on 14.09.2026.
 * Copyright (c) 2024-2026, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include <Vortex/Core/Types.hpp>

import Prism.Containers.Array;
import Prism.String.String;

namespace Vortex::Editor
{
    class EditorContext;

    /**
     * The settings window (Edit > Settings, Ctrl+,): project, editor,
     * graphics (GPU choice and info) and input (shortcut and camera key
     * mapping) pages. Editor preferences save when it closes; project
     * settings save with "Save Project".
     */
    class SettingsOverlay
    {
      public:
        /// Draws it while context.ShowSettings is set; clears it on close.
        void Render(EditorContext& context);

      private:
        enum class Page : u8
        {
            eProjectGeneral,
            eProjectRenderer,
            eEditorGeneral,
            eAppearance,
            eViewport,
            eGraphics,
            eShortcuts,
            eCameraControls,
        };
        /// What a pressed key is being captured for.
        enum class Capture : u8
        {
            eNone,
            /// A command's chord (modifiers + key).
            eCommand,
            /// A camera movement key (a single key; modifier keys allowed).
            eCameraKey,
        };

        static constexpr usize kFrameHistory = 160;

        Page                   m_Page        = Page::eGraphics;
        bool                   m_WasOpen     = false;
        String                 m_ShortcutFilter;

        Capture                m_Capture        = Capture::eNone;
        String                 m_CaptureCommand;
        i32*                   m_CaptureKey     = nullptr;
        /// Frame a capture ended: the key that ended it may also "click" the
        /// focused button (Space/Enter) or close the window (Escape).
        i32                    m_CaptureEndedOn = -1;

        String                 m_Status;
        f64                    m_StatusTime = 0.0;

        /// The GPU and API preferences the renderer started with, to tell
        /// when a restart is needed.
        String                 m_StartupGpu;
        String                 m_StartupAPI;
        bool                   m_HaveStartupGpu = false;

        bool                   NeedsRestart(const EditorContext& context) const;

        Array<f32, kFrameHistory> m_FrameTimes{};
        usize                  m_FrameOffset = 0;

        void                   OnOpen(EditorContext& context);
        void                   OnClose(EditorContext& context);

        void                   DrawHeader(EditorContext& context);
        void                   DrawSidebar(EditorContext& context);
        void                   DrawPage(EditorContext& context);
        void                   DrawFooter(EditorContext& context);

        void                   DrawProjectGeneralPage(EditorContext& context);
        void                   DrawProjectRendererPage(EditorContext& context);
        void                   DrawEditorGeneralPage(EditorContext& context);
        void                   DrawAppearancePage(EditorContext& context);
        void                   DrawViewportPage(EditorContext& context);
        void                   DrawGraphicsPage(EditorContext& context);
        void                   DrawShortcutsPage(EditorContext& context);
        void                   DrawCameraControlsPage(EditorContext& context);

        /// A button showing `chord`; click to rebind it.
        bool                   DrawBindingButton(const char* id, i32 chord, bool capturing,
                                                 f32 width);
        void                   UpdateCapture(EditorContext& context);
        void                   EndCapture();
        void                   SetStatus(String status);
        void                   RecordFrameTime();
    };
} // namespace Vortex::Editor
