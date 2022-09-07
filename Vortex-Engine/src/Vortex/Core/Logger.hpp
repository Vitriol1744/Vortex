//
// Created by Vitriol1744 on 05.09.2022.
//
#ifndef VORTEX_LOGGER_HPP
#define VORTEX_LOGGER_HPP

#include "Vortex/Core/Export.hpp"

#include <string>

namespace Vortex
{
    enum class LogLevel
    {
        eTrace,
        eInfo,
        eWarn,
        eError,
        eFatal
    };

    class VT_API Logger
    {
        public:
            inline explicit Logger(std::string_view name) : name(name) { }

            void Log(LogLevel log_level, std::string_view formatted_message);
            void SetName(std::string_view name) { this->name = name; }

        private:
            std::string name;
    };
}


#endif //VORTEX_LOGGER_HPP
