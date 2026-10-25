/*
 * Created by v1tr10l7 on 08.09.2026.
 * Copyright (c) 2024-2026, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include <Prism/Core/Version.hpp>
#include <Prism/String/String.hpp>
#include <Prism/Utility/Path.hpp>

namespace Vortex
{
    class ArchiveWriter;
    class ArchiveReader;

    template <typename T>
    concept CanSerialize = requires(const T& obj, ArchiveWriter& writer) {
        { obj.Serialize(writer) } -> SameAs<void>;
    };

    template <typename T>
    concept CanDeserialize = requires(T& obj, ArchiveReader& reader) {
        { obj.Deserialize(reader) } -> SameAs<bool>;
    };

    template <typename T>
    concept IsSerializable = CanSerialize<T> && CanDeserialize<T>;

    class ArchiveWriter
    {
      public:
        virtual ~ArchiveWriter()                              = default;

        // Structured Nodes (for formats like JSON/YAML; no-ops or layout tags
        // for Binary)
        virtual void BeginObject(StringView name)             = 0;
        virtual void EndObject()                              = 0;

        virtual void BeginArray(StringView name, usize count) = 0;
        virtual void EndArray()                               = 0;

        // Primitive Serialization
        virtual void Write(StringView key, bool value)        = 0;
        virtual void Write(StringView key, i32 value)         = 0;
        virtual void Write(StringView key, i64 value)         = 0;
        virtual void Write(StringView key, u32 value)         = 0;
        virtual void Write(StringView key, u64 value)         = 0;
        virtual void Write(StringView key, f32 value)         = 0;
        virtual void Write(StringView key, f64 value)         = 0;
        virtual void Write(StringView key, StringView value)  = 0;
        virtual void Write(StringView key, PathView value)    = 0;
    };

    class ArchiveReader
    {
      public:
        virtual ~ArchiveReader()                                  = default;

        virtual bool BeginObject(StringView name)                 = 0;
        virtual void EndObject()                                  = 0;

        virtual bool BeginArray(StringView name, usize& outCount) = 0;
        virtual bool EndArray()                                   = 0;

        // Primitive Deserialization
        virtual bool Read(StringView key, bool& outValue)         = 0;
        virtual bool Read(StringView key, i32& outValue)          = 0;
        virtual bool Read(StringView key, i64& outValue)          = 0;
        virtual bool Read(StringView key, u32& outValue)          = 0;
        virtual bool Read(StringView key, u64& outValue)          = 0;
        virtual bool Read(StringView key, f32& outValue)          = 0;
        virtual bool Read(StringView key, f64& outValue)          = 0;
        virtual bool Read(StringView key, String& outValue)       = 0;
        virtual bool Read(StringView key, Path& outValue)         = 0;
    };

    template <CanSerialize T>
    inline void WriteObject(ArchiveWriter& writer, StringView name,
                            const T& object)
    {
        writer.BeginObject(name);
        object.Serialize(writer);
        writer.EndObject();
    }

    template <CanDeserialize T>
    inline bool ReadObject(ArchiveReader& reader, StringView name, T& outObject)
    {
        if (!reader.BeginObject(name)) return false;

        bool result = outObject.Deserialize(reader);
        reader.EndObject();
        return result;
    }

    template <typename T>
    void SerializeValue(ArchiveWriter& writer, StringView key, const T& value)
    {
        writer.BeginObject(key);
        value.Serialize(writer);
        writer.EndObject();
    }
    template <typename T>
    bool DeserializeValue(ArchiveReader& reader, StringView key, T& outValue)
    {
        if (!reader.BeginObject(key)) return false;

        bool result = outValue.Deserialize(reader);
        reader.EndObject();
        return result;
    }

    inline void SerializeValue(ArchiveWriter& writer, StringView key,
                               const Version& version)
    {
        writer.Write(key, version.operator String().View());
    }
    inline bool DeserializeValue(ArchiveReader& reader, StringView key,
                                 Version& outVersion)
    {
        String versionString;
        if (reader.Read(key, versionString))
        {
            u16 major = 0, minor = 0, patch = 0;
            if (std::sscanf(versionString.Raw(), "%hu.%hu.%hu", &major, &minor,
                            &patch)
                == 3)
            {
                outVersion = Version{major, minor, patch};
                return true;
            }
        }
        return false;
    }
} // namespace Vortex
