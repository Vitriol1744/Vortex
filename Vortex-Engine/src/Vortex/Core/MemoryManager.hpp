//
// Created by vitriol1744 on 21.07.2021.
//
#pragma once

#include "Core/Core.hpp"

namespace Vortex
{
    class VT_API MemoryManager : public Singleton<MemoryManager>
    {
        public:
            void* Allocate(size_t size);
            void Deallocate(void* ptr, size_t size);

            inline size_t GetUsage() const noexcept { return totalAllocated - totalFreed; }

        private:
            size_t totalAllocated = 0;
            size_t totalFreed = 0;
    };
}