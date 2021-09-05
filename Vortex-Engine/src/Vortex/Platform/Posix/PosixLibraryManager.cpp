//
// Created by vitriol1744 on 27.08.2021.
//
#include "Vortex/Platform/LibraryManager.hpp"

#ifdef VT_PLATFORM_LINUX
#include <unistd.h>
#include <dlfcn.h>

namespace Vortex::Platform
{
    void* LibraryManager::LoadLibrary(const char* filename)
    {
        char* library_FileName = new char[strlen(filename) + 1 + 3];
        std::strcpy(library_FileName, filename);
        std::strcat(library_FileName, ".so");

        void* library = dlopen(filename, RTLD_LAZY);

        return library;
    }
    void LibraryManager::CloseLibrary(void* library)
    {
        dlclose(library);
    }
    
    void* LibraryManager::GetProcAddress(void* library, const char* procname)
    {
        void* proc = dlsym(library, procname);
        
        return proc;
    }
}
#endif