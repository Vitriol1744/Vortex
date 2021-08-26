//
// Created by vitriol1744 on 21.07.2021.
//
#include "MemoryManager.hpp"

using namespace Vortex;

namespace Vortex
{
    void* MemoryManager::Allocate(size_t size)
    {
        totalAllocated += size;
        return malloc(size);
    }

    void MemoryManager::Deallocate(void* ptr, size_t size)
    {
        totalFreed += size;
        free(ptr);
    }
}

void* operator new(size_t size)
{
    return MemoryManager::Instance()->Allocate(size);
}

void operator delete(void* ptr, size_t size)
{
    MemoryManager::Instance()->Deallocate(ptr, size);
    ptr = nullptr;
}