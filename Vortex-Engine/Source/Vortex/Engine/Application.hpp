/*
 * Created by v1tr10l7 on 17.07.2024.
 * Copyright (c) 2024-2024, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include <Prism/Core/Version.hpp>

#include <Vortex/Core/Events/EventBus.hpp>
#include <Vortex/Core/LayerStack.hpp>

#define VT_ENGINE_API __attribute__((visibility("default")))

namespace Vortex
{
    class AssetManager;
    class ImGuiLayer;
    class RenderTargetWindow;

    struct ApplicationSpecification
    {
        String Name = "Vortex Application";
    };

    // TODO(v1tr10l7): Implement command line arguments
    class CommandLineArguments
    {
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
        ::Ref<AssetManager>              AssetManager();

        static Application*              Get() { return s_Instance; }

      private:
        bool                            m_Running       = false;
        bool                            m_ShouldRestart = false;
        String                          m_Name;
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

    // To be defined by Client
    extern VT_API Application* CreateApplication(const CommandLineArguments&);
}; // namespace Vortex
