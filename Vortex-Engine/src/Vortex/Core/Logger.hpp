//
// Created by Vitriol1744 on 24.06.2021.
//
#pragma once

#include "Vortex/Core/Export.hpp"
#include "Vortex/Core/PlatformInit.hpp"

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
            Logger() = default;
            explicit Logger(std::string_view name) { }
            virtual ~Logger() = default;
            
            static void Initialize(); //NOTE: Has to be implemented by derived class
            virtual void Log(LogLevel level, std::string_view formattedString) = 0;
            
            void SetName(std::string_view name) { this->name = name; }
            
        protected:
            std::string name;
    };
}

