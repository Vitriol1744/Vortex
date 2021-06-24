//
// Created by Vitriol1744 on 24.06.2021.
//

#pragma once

#include "Core/Logger.hpp"

namespace Vortex
{
    class VT_API LoggerImpl : public Logger
    {
        protected:
            void InitializeImpl() override;
            void LogImpl(LogLevel level, std::string_view formattedString) override;
    };
}


