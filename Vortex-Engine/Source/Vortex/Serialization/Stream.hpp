/*
 * Created by v1tr10l7 on 13.09.2026.
 * Copyright (c) 2024-2026, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

import Prism.String.String;

namespace Vortex
{
    class StreamReader
    {
      public:
        virtual ~StreamReader()                               = default;

        virtual bool Good() const                             = 0;
        virtual u64  Position() const                         = 0;
        virtual bool SetPosition(u64 position)                = 0;
        virtual bool ReadBytes(char* destination, usize size) = 0;

        inline       operator bool() const { return Good(); }

        // Reads trivially copyable types (PODs, ints, floats, simple structs)
        template <typename T>
            requires IsTriviallyCopyableV<T>
        bool Read(T& value)
        {
            return ReadBytes(reinterpret_cast<char*>(&value), sizeof(T));
        }

        // Bulk-reads contiguous array elements directly into buffer
        template <typename T>
            requires IsTriviallyCopyableV<T>
        bool ReadArray(Vector<T>& array, u32 count)
        {
            array.Resize(count);
            if (count == 0) return true;
            return ReadBytes(reinterpret_cast<char*>(array.Raw()),
                             count * sizeof(T));
        }

        bool ReadString(String outString)
        {
            u32 length = 0;
            if (!Read(length)) return false;

            outString.Resize(length);
            if (length == 0) return true;

            return ReadBytes(outString.Raw(), length);
        }
    };

    class StreamWriter
    {
      public:
        virtual ~StreamWriter()                               = default;

        virtual bool Good() const                             = 0;
        virtual u64  Position() const                         = 0;
        virtual bool SetPosition(u64 position)                = 0;
        virtual bool WriteBytes(const char* data, usize size) = 0;

        inline       operator bool() const { return Good(); }

        // Writes trivially copyable types
        template <typename T>
            requires IsTriviallyCopyableV<T>
        bool Write(const T& value)
        {
            return WriteBytes(reinterpret_cast<const char*>(&value), sizeof(T));
        }

        // Bulk-writes contiguous array elements
        template <typename T>
            requires IsTriviallyCopyableV<T>
        bool WriteArray(const Vector<T>& array, bool writeCount = true)
        {
            if (writeCount)
            {
                u32 count = static_cast<u32>(array.Size());
                if (!Write(count)) return false;
            }

            if (array.Empty()) return true;
            return WriteBytes(reinterpret_cast<const char*>(array.Raw()),
                              array.Size() * sizeof(T));
        }

        bool WriteString(const StringView string)
        {
            u32 length = static_cast<u32>(string.Size());
            if (!Write(length)) return false;
            if (length == 0) return true;

            return WriteBytes(string.Raw(), length);
        }
    };
} // namespace Vortex
