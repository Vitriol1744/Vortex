/*
 * Created by v1tr10l7 on 17.11.2026.
 * Copyright (c) 2024-2026, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
export module Nexus.EditorWindow;

import Prism.Containers.Vector;
import Prism.Core.TypeTraits;
import Prism.Memory.Ref;
import Prism.Memory.RefCounted;
import Prism.String.String;
import Prism.String.StringView;

// The editor's shared state and projects; regular headers (they carry
// scene/entt types).
export extern "C++"
{
    namespace Vortex::Editor
    {
        class EditorContext;
        class EditorProject;
    }
}

export namespace Vortex::Editor
{
    using Prism::Ref;
    using Prism::String;
    using Prism::StringView;
    using Prism::Vector;

    /**
     * A dockable editor window. The WindowManager owns the ImGui window
     * (Begin/End, the close button, the open state, its menu toggle and its
     * persistence); subclasses only draw its contents in OnDraw.
     *
     * Adding a window to the editor:
     *
     *   class MyWindow final : public EditorWindow
     *   {
     *     public:
     *       MyWindow() : EditorWindow("My Window", "Tools") {}
     *       void OnDraw(EditorContext& context) override { ... }
     *   };
     *
     * and one line in RegisterEditorWindows (UI/EditorWindows.cpp). It
     * then shows up in its menu, docks, and remembers whether it was open.
     */
    class EditorWindow : public Prism::RefCounted<EditorWindow>
    {
      public:
        /// `title` is also the ImGui window name, so it keys the layout in
        /// imgui.ini. `menu` is the top-level menu listing the window.
        explicit EditorWindow(StringView title, StringView menu = "View")
            : m_Title(title)
            , m_Menu(menu)
        {
        }
        virtual ~EditorWindow() = default;

        StringView   Title() const { return m_Title; }
        StringView   Menu() const { return m_Menu; }

        bool         Open     = true;
        /// Has a close button (otherwise only the menu hides it).
        bool         Closable = true;
        /// ImGuiWindowFlags.
        int          Flags    = 0;
        /// OnDraw makes its own ImGui window(s) (e.g. ImGui's demo, which
        /// takes `&Open`); the manager then skips Begin/End.
        bool         SelfDrawn = false;

        /// Once, right after the manager adopts the window.
        virtual void OnAttach(EditorContext& context) { (void)context; }
        /// Every frame, open or not (background work).
        virtual void OnUpdate(EditorContext& context, float deltaTime)
        {
            (void)context;
            (void)deltaTime;
        }
        /// Every frame while open, in the layers' render phase: record
        /// render-graph work here (never inside OnDraw).
        virtual void OnRender(EditorContext& context) { (void)context; }
        /// Contents, between the manager's Begin and End.
        virtual void OnDraw(EditorContext& context) = 0;
        /// After another project became the active one.
        virtual void OnProjectChanged(EditorContext& context,
                                      EditorProject& project)
        {
            (void)context;
            (void)project;
        }
        /// Around Begin: SetNextWindow*, Push/PopStyleVar...
        virtual void OnBeforeBegin(EditorContext& context) { (void)context; }
        virtual void OnAfterBegin(EditorContext& context) { (void)context; }

      private:
        String m_Title;
        String m_Menu;
    };

    class WindowManager
    {
      public:
        WindowManager() = default;
        ~WindowManager();
        WindowManager(const WindowManager&)            = delete;
        WindowManager& operator=(const WindowManager&) = delete;

        /// Creates and adopts a window; it lives as long as the manager.
        template <typename T, typename... Args>
        T& Add(EditorContext& context, Args&&... args)
        {
            auto window = Prism::CreateRef<T>(Prism::Forward<Args>(args)...);
            m_Windows.PushBack(window);
            window->OnAttach(context);
            return *window;
        }

        /// The first window of type T, or null.
        template <typename T>
        Ref<T> Get() const
        {
            for (const auto& window : m_Windows)
                if (auto* typed = dynamic_cast<T*>(window.Raw()))
                    return Ref<T>(typed);
            return nullptr;
        }
        EditorWindow* Find(StringView title) const;

        void          Update(EditorContext& context, float deltaTime);
        void          Render(EditorContext& context);
        /// Every open window, in registration order.
        void          Draw(EditorContext& context);
        void          NotifyProjectChanged(EditorContext& context,
                                           EditorProject& project);
        /// Toggles for the windows listed under `menu`; false if there are
        /// none.
        bool          DrawMenuItems(StringView menu);
        bool          HasMenuItems(StringView menu) const;

        /// Persists open states in imgui.ini. Call before the first ImGui
        /// frame (settings load then).
        void          RegisterSettingsHandler();

        const Vector<Ref<EditorWindow>>& Windows() const { return m_Windows; }

      private:
        Vector<Ref<EditorWindow>> m_Windows;
        bool                      m_HandlerRegistered = false;
    };
} // namespace Vortex::Editor
