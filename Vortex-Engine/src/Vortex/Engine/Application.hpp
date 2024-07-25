/*
 * Created by v1tr10l7 on 17.07.2024.
 * Copyright (c) 2024-2024, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include "Vortex/Core/Export.hpp"
#include "Vortex/Graphics/Window/Window.hpp"

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
         * restarted uppon close
         */
        bool                    Run();
        /**
         * @brief Closes the application
         */
        void                    Close();

        inline std::string_view GetName() const { return m_Name; }
        inline const Version&   GetVersion() const { return m_Version; }
        inline Ref<Window>      GetWindow() { return m_MainWindow; }

        static Application*     Get() { return s_Instance; }

      private:
        bool                m_Running       = false;
        bool                m_ShouldRestart = false;
        std::string         m_Name;
        Version             m_Version    = {0, 1, 0};
        Ref<Window>         m_MainWindow = nullptr;

        static Application* s_Instance;
    };

    // To be defined by Client
    extern Application* CreateApplication(const CommandLineArguments&);
}; // namespace Vortex
