//
// Created by vitriol1744 on 04.07.2021.
//
#include "LoggingManager.hpp"

namespace Vortex
{
    LoggingManager::LoggingManager()
    {
        coreLogger  = CreateRef<Logger>("Vortex");
        logger      = CreateRef<Logger>("Client");
    }
}
