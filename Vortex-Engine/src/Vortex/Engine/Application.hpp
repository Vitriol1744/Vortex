/*
 * Created by v1tr10l7 on 17.07.2024.
 * Copyright (c) 2024-2024, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include "Vortex/Core/LayerStack.hpp"
#include "Vortex/Renderer/API/Vulkan/VulkanImGuiLayer.hpp"
#include "Vortex/Window/Window.hpp"

namespace Vortex
{
    struct ApplicationSpecification
    {
        std::string Name = "Vortex Application";
    };

    // TODO(v1tr10l7): Implement command line arguments
    class CommandLineArguments
    {
    };

    class VT_API Application
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
        bool        Run();
        /**
         * @brief Closes the application
         */
        void        Close();
        /**
         * @brief Restart the application
         */
        void        Restart();

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

        inline std::string_view GetName() const { return m_Name; }
        inline const Version&   GetVersion() const { return m_Version; }
        inline Window&          GetWindow() { return *m_MainWindow; }
        inline u64              GetFPSCounter() { return m_FpsCounter; }
        inline f64              GetDeltaTime() { return m_DeltaTime; }

        static Application*     Get() { return s_Instance; }

      private:
        bool                  m_Running       = false;
        bool                  m_ShouldRestart = false;
        std::string           m_Name;
        Version               m_Version    = {0, 1, 0};
        Scope<Window>         m_MainWindow = nullptr;
        u64                   m_FpsCounter = 0;
        f64                   m_DeltaTime  = 0.0;

        LayerStack            m_LayerStack;
        Ref<VulkanImGuiLayer> m_ImGuiLayer = nullptr;

        static Application*   s_Instance;

        void                  RenderImGui();
    };

    // To be defined by Client
    extern Application* CreateApplication(const CommandLineArguments&);
}; // namespace Vortex
