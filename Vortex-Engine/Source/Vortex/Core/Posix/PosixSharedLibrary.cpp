/*
 * Created by v1tr10l7 on 14.06.2026.
 * Copyright (c) 2024-2026, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#include <Vortex/Core/SharedLibrary.hpp>
#include <dlfcn.h>

namespace Vortex
{
    SharedLibrary::SharedLibrary(PathView path) { Load(path); }

    SharedLibrary::~SharedLibrary() { Unload(); }

    SharedLibrary::SharedLibrary(SharedLibrary&& other) noexcept
        : m_Handle(other.m_Handle)
        , m_LastError(Move(other.m_LastError))
    {
        other.m_Handle = nullptr;
    }

    SharedLibrary& SharedLibrary::operator=(SharedLibrary&& other) noexcept
    {
        if (this != &other)
        {
            Unload();
            m_Handle       = other.m_Handle;
            m_LastError    = Move(other.m_LastError);
            other.m_Handle = nullptr;
        }
        return *this;
    }

    bool SharedLibrary::Load(PathView path)
    {
        Unload();

        String nullTerminatedPath(path);
        // RTLD_LAZY binds symbols as they are evaluated
        m_Handle = dlopen(nullTerminatedPath.Raw(), RTLD_LAZY);

        if (!m_Handle)
        {
            const char* errorString = dlerror();
            m_LastError
                = errorString ? errorString : "Unknown POSIX loading error.";
            return false;
        }

        m_LastError.Clear();
        return true;
    }

    void SharedLibrary::Unload()
    {
        if (m_Handle)
        {
            dlclose(m_Handle);
            m_Handle = nullptr;
        }
    }

    void* SharedLibrary::SymbolAddress(StringView name) const
    {
        if (!m_Handle) return nullptr;

        String nullTerminatedName(name);
        return dlsym(m_Handle, nullTerminatedName.Raw());
    }
} // namespace Vortex
