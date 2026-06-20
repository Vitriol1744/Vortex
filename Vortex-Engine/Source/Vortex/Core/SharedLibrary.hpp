/*
 * Created by v1tr10l7 on 14.06.2026.
 * Copyright (c) 2024-2026, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once
#pragma once

#include <Prism/Utility/Path.hpp>

namespace Vortex
{
    class SharedLibrary
    {
      public:
        SharedLibrary() = default;
        explicit SharedLibrary(PathView path);
        ~SharedLibrary();

        // Prevent copying to avoid double-freeing the handle
        SharedLibrary(const SharedLibrary&)            = delete;
        SharedLibrary& operator=(const SharedLibrary&) = delete;

        // Allow movement
        SharedLibrary(SharedLibrary&& other) PM_NOEXCEPT;
        SharedLibrary&    operator=(SharedLibrary&& other) PM_NOEXCEPT;

        bool              Load(PathView path);
        void              Unload();

        PM_NODISCARD bool IsLoaded() const { return m_Handle != nullptr; }
        PM_NODISCARD const String& LastError() const { return m_LastError; }

        template <typename T>
        T Function(StringView name) const
        {
            return reinterpret_cast<T>(SymbolAddress(name));
        }

      private:
        void*  SymbolAddress(StringView name) const;

        void*  m_Handle = nullptr;
        String m_LastError;
    };
} // namespace Vortex
