//
// Created by Vitriol1744 on 24.06.2021.
//
#pragma once

#include "Export.hpp"
#include "Macros.hpp"

#include <string>
#include "fmt/format.h"

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
            Logger() = default;
            explicit Logger(std::string_view name) { }
            virtual ~Logger() = default;
            
            static void Initialize(); //NOTE: Has to be implemented by derived class
            virtual void Log(LogLevel level, std::string_view formattedString) = 0;
            
            void SetName(std::string_view name) { this->name = std::string("[") + std::string(name) + "]: "; }
            
        protected:
            std::string name;
    };
}

