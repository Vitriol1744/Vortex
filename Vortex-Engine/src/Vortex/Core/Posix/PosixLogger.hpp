//
// Created by Vitriol1744 on 29.06.2021.
//
#pragma once

#include "Vortex/Core/Logger.hpp"

namespace Vortex
{
    class VT_API LoggerImpl final : public Logger
    {
        public:
            inline explicit LoggerImpl(std::string_view name) { SetName(name); }
            
            ~LoggerImpl() override = default;
        
            void Log(LogLevel level, std::string_view formattedString) override;
    };
}