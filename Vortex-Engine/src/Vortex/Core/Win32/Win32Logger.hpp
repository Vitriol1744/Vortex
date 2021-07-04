//
// Created by Vitriol1744 on 24.06.2021.
//
#pragma once

#include "Core/Logger.hpp"

namespace Vortex
{
    class VT_API LoggerImpl : public Logger
    {
        public:
            inline explicit LoggerImpl(std::string_view name) { this->name = name; }
        
            ~LoggerImpl() override = default;
        
            void Log(LogLevel level, std::string_view formattedString) override;
    };
}


