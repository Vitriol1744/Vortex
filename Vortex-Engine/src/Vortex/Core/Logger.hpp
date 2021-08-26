//
// Created by Vitriol1744 on 24.06.2021.
//
#pragma once

#include "Vortex/Core/Export.hpp"
#include "Vortex/Platform/Platform.hpp"

#include <string>

namespace Vortex
{
    enum class LogLevel
    {
        Trace,
        Info,
        Warn,
        Error,
        Fatal
    };
    
    class VT_API Logger
    {
        public:
            Logger(std::string_view name) { SetName(name); }
            
            void Log(LogLevel level, std::string_view formattedString);
            
            void SetName(std::string_view name) { this->name = name; }
            
        private:
            std::string name;
    };
}

