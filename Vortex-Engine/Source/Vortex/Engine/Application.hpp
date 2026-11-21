/*
 * Created by v1tr10l7 on 17.07.2024.
 * Copyright (c) 2024-2024, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

import Prism.Containers.Vector;
import Prism.Core.Version;
import Prism.String.String;
import Prism.String.StringView;

#include <Vortex/Core/Events/EventBus.hpp>
#include <Vortex/Core/LayerStack.hpp>

#define VT_ENGINE_API __attribute__((visibility("default")))

namespace Vortex
{
    class AssetManager;
    class ImGuiLayer;
    class RenderTargetWindow;

    /// The process's command line (argv[0] included).
    class CommandLineArguments
    {
      public:
        CommandLineArguments() = default;
        CommandLineArguments(int argc, char** argv)
        {
            for (int i = 0; i < argc; ++i) m_Args.PushBack(String(argv[i]));
        }

        inline usize      Count() const { return m_Args.Size(); }
        inline StringView operator[](usize index) const
        {
            return m_Args[index].View();
        }

        /// True when `flag` (e.g. "--verbose") appears.
        bool Has(StringView flag) const
        {
            for (const auto& arg : m_Args)
                if (arg.View() == flag) return true;
            return false;
        }
        /// The argument after `flag` ("--project <path>"); empty if `flag`
        /// is missing or last.
        StringView Value(StringView flag) const
        {
            for (usize i = 0; i + 1 < m_Args.Size(); ++i)
                if (m_Args[i].View() == flag) return m_Args[i + 1].View();
            return {};
        }

      private:
        Vector<String> m_Args;
    };

    /// Engine systems an application may opt out of. Tools that only need a
    /// window and ImGui (the Horizon launcher) turn these off, so they don't
    /// load the .NET runtime or PhysX.
    struct ApplicationSubsystems
    {
        bool SceneRenderer = true;
        bool Physics       = true;
        bool Scripting     = true;
    };

    struct ApplicationSpecification
    {
        String                Name = "Vortex Application";
        CommandLineArguments  Arguments;
        ApplicationSubsystems Subsystems;
        u32                   WindowWidth  = 800;
        u32                   WindowHeight = 600;
        /// Main window presentation; can be changed later on its swapchain.
        bool                  VSync        = false;
        /// GPU to run on, by name; empty picks the best one.
        String                PreferredAdapter{};
        /// Graphics API by name ("Vulkan"); empty or unknown uses the
        /// default.
        String                RendererAPI{};
    };

    class VT_ENGINE_API Application
    {
      public:
        /**
         * @brief Constructs the application
         */
        Application(const ApplicationSpecification& specification);
        /**
         * @brief Destructs the application
         */
        virtual ~Application();

        /**
         * @brief Runs the application
         * @return boolean value indicating, whether application should be
         * restarted upon close
         */
        bool Run();
        /**
         * @brief Closes the application
         */
        void Close();
        /**
         * @brief Restart the application
         */
        void Restart();

        template <IsEvent T, typename... Args>
        void PublishEvent(Args&&... args)
        {
            m_EventBus.PublishEvent<T>(Forward<Args>(args)...);
        }
        template <IsEvent T, typename F>
        void SubscribeEvent(F&& cb)
        {
            m_EventBus.Subscribe<T>(Forward<F>(cb));
        }

        inline void PushOverlay(Ref<Layer> overlay)
        {
            return m_LayerStack.PushOverlay(overlay);
        }
        inline void PushLayer(Ref<Layer> layer)
        {
            return m_LayerStack.PushLayer(layer);
        }

        inline Ref<Layer> PopOverlay(Ref<Layer> overlay)
        {
            return m_LayerStack.PopOverlay(overlay);
        }
        inline Ref<Layer> PopLayer(Ref<Layer> layer)
        {
            return m_LayerStack.PopLayer(layer);
        }

        inline StringView                Name() const { return m_Name; }
        inline const struct Version&     Version() const { return m_Version; }
        inline class RenderTargetWindow& Window() { return *m_MainWindow; }
        inline u64                       FPSCounter() { return m_FpsCounter; }
        inline f64                       DeltaTime() { return m_DeltaTime; }
        inline const CommandLineArguments& Arguments() const
        {
            return m_Arguments;
        }
        ::Ref<AssetManager>              AssetManager();

        static Application*              Get() { return s_Instance; }

      private:
        bool                            m_Running       = false;
        bool                            m_ShouldRestart = false;
        String                          m_Name;
        CommandLineArguments            m_Arguments;
        ApplicationSubsystems           m_Subsystems;
        struct Version                  m_Version = {0, 1, 0};
        EventBus                        m_EventBus;
        Scope<class RenderTargetWindow> m_MainWindow = nullptr;
        u64                             m_FpsCounter = 0;
        f64                             m_DeltaTime  = 0.0;

        LayerStack                      m_LayerStack;
        ::Ref<ImGuiLayer>               m_ImGuiLayer;
        ::Ref<class AssetManager>       m_AssetManager;

        static Application*             s_Instance;

        void Initialize(const ApplicationSpecification& specification);
        void Shutdown();

        void RenderImGui();
    };

    /**
     * What CreateApplication returns: the application to run, or no
     * application and the process's exit code (a headless command, like
     * `Horizon --install`, or a failed precondition).
     */
    struct ApplicationLaunch
    {
        Scope<Application> App;
        int                ExitCode = 0;

        ApplicationLaunch(Scope<Application> app)
            : App(Move(app))
        {
        }
        template <typename T>
        ApplicationLaunch(Scope<T>&& app)
            : App(Move(app))
        {
        }

        /// Ends the program without running an application.
        static ApplicationLaunch Exit(int exitCode)
        {
            ApplicationLaunch launch{Scope<Application>()};
            launch.ExitCode = exitCode;
            return launch;
        }
    };

    /**
     * Defined by each program (Nexus, Horizon, a game): builds its
     * application from the command line. The entry point runs it, and calls
     * this again when the application asks for a restart.
     */
    ApplicationLaunch CreateApplication(const CommandLineArguments& args);
}; // namespace Vortex
