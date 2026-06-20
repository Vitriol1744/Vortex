/*
 * Created by v1tr10l7 on 14.06.2026.
 * Copyright (c) 2024-2026, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#include <Vortex/Core/SharedLibrary.hpp>
#include <Vortex/Core/Win32/Windows.hpp>

namespace Vortex
{
    SharedLibrary::SharedLibrary(StringView path) { Load(path); }

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

        // Win32 requires a null-terminated string
        String nullTerminatedPath(path);
        m_Handle = static_cast<void*>(LoadLibraryA(nullTerminatedPath.Raw()));

        if (!m_Handle)
        {
            DWORD errorMessageID = ::GetLastError();
            if (errorMessageID != 0)
            {
                LPSTR messageBuffer = nullptr;
                usize size          = FormatMessageA(
                    FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM
                        | FORMAT_MESSAGE_IGNORE_INSERTS,
                    nullptr, errorMessageID,
                    MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                    reinterpret_cast<LPSTR>(&messageBuffer), 0, nullptr);

                m_LastError = String(messageBuffer, size);
                LocalFree(messageBuffer);
            }
            return false;
        }

        m_LastError.Clear();
        return true;
    }

    void SharedLibrary::Unload()
    {
        if (m_Handle)
        {
            FreeLibrary(static_cast<HMODULE>(m_Handle));
            m_Handle = nullptr;
        }
    }

    void* SharedLibrary::SymbolAddress(StringView name) const
    {
        if (!m_Handle) return nullptr;

        String nullTerminatedName(name);
        return reinterpret_cast<void*>(GetProcAddress(
            static_cast<HMODULE>(m_Handle), nullTerminatedName.Raw()));
    }
} // namespace Vortex
