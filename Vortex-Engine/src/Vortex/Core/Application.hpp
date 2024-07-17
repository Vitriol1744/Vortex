/*
 * Created by v1tr10l7 on 17.07.2024.
 * Copyright (c) 2024-2024, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include "Vortex/Core/Export.hpp"

#include <string>

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
        bool Run();
        /**
         * @brief Closes the application
         */
        void Close();

      private:
        bool                m_Running       = false;
        bool                m_ShouldRestart = false;
        std::string         m_Name;

        static Application* s_Instance;
    };

    // To be defined by Client
    extern Application* CreateApplication(const CommandLineArguments&);
}; // namespace Vortex
