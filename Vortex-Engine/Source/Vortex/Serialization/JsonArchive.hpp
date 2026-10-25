/*
 * Created by v1tr10l7 on 08.09.2026.
 * Copyright (c) 2024-2026, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include <Vortex/Serialization/Archive.hpp>

#include <fstream>
#include <nlohmann/json.hpp>
#include <sstream>

namespace Vortex
{
    class JsonArchiveWriter : public ArchiveWriter
    {
      public:
        JsonArchiveWriter() { m_NodeStack.PushBack(&m_Root); }

        void BeginObject(StringView name) override
        {
            nlohmann::ordered_json& current = *m_NodeStack.Back();
            if (current.is_array())
            {
                current.push_back(nlohmann::ordered_json::object());
                m_NodeStack.PushBack(&current.back());
            }
            else {
                std::string key(name.Raw(), name.Size());
                current[key] = nlohmann::ordered_json::object();
                m_NodeStack.PushBack(&current[key]);
            }
        }
        void BeginObject()
        {
            nlohmann::ordered_json& current = *m_NodeStack.Back();

            if (current.is_array())
            {
                current.push_back(nlohmann::ordered_json::object());
                m_NodeStack.PushBack(&current.back());
            }
            else {
                // Fallback for non-array usage without name
                current.push_back(nlohmann::ordered_json::object());
                m_NodeStack.PushBack(&current.back());
            }
        }

        void EndObject() override { m_NodeStack.PopBack(); }

        void BeginArray(StringView name, usize count) override
        {
            nlohmann::ordered_json& current = *m_NodeStack.Back();

            if (current.is_array())
            {
                current.push_back(nlohmann::ordered_json::array());
                m_NodeStack.PushBack(&current.back());
            }
            else {
                std::string key(name.Raw(), name.Size());
                current[key] = nlohmann::ordered_json::array();
                m_NodeStack.PushBack(&current[key]);
            }
        }
        void EndArray() override { m_NodeStack.PopBack(); }

        void Write(StringView key, bool value) override
        {
            (*m_NodeStack.Back())[std::string_view(key.Raw(), key.Size())]
                = value;
        }
        void Write(StringView key, i32 value) override
        {
            (*m_NodeStack.Back())[std::string_view(key.Raw(), key.Size())]
                = value;
        }
        void Write(StringView key, i64 value) override
        {
            (*m_NodeStack.Back())[std::string_view(key.Raw(), key.Size())]
                = value;
        }

        void Write(StringView key, u32 value) override
        {
            (*m_NodeStack.Back())[std::string_view(key.Raw(), key.Size())]
                = value;
        }
        void Write(StringView key, u64 value) override
        {
            (*m_NodeStack.Back())[std::string_view(key.Raw(), key.Size())]
                = value;
        }

        void Write(StringView key, f32 value) override
        {
            (*m_NodeStack.Back())[std::string_view(key.Raw(), key.Size())]
                = value;
        }
        void Write(StringView key, f64 value) override
        {
            (*m_NodeStack.Back())[std::string_view(key.Raw(), key.Size())]
                = value;
        }

        void Write(StringView key, StringView value) override
        {
            (*m_NodeStack.Back())[std::string_view(key.Raw(), key.Size())]
                = std::string_view(value.Raw(), value.Size());
        }
        void Write(StringView key, PathView value) override
        {
            (*m_NodeStack.Back())[std::string_view(key.Raw(), key.Size())]
                = value.Raw();
        }

        String ToString() const
        {
            auto view = m_Root.dump(2);
            return String(view.data(), view.size());
        }

      private:
        nlohmann::ordered_json          m_Root;
        Vector<nlohmann::ordered_json*> m_NodeStack;
    };
    class JsonArchiveReader : public ArchiveReader
    {
      public:
        JsonArchiveReader() = default;

        explicit JsonArchiveReader(StringView jsonString) { Parse(jsonString); }

        bool Parse(StringView jsonString)
        {
            try
            {
                m_Root = nlohmann::ordered_json::parse(
                    std::string_view(jsonString.Raw(), jsonString.Size()));
                m_NodeStack.Clear();
                m_ArrayIndexStack.Clear();

                m_NodeStack.PushBack(&m_Root);
                m_ArrayIndexStack.PushBack(0);
                return true;
            }
            catch (const nlohmann::ordered_json::parse_error& e)
            {
                VtCoreError("JsonArchiveReader::Parse exception: {}", e.what());
                return false;
            }
        }

        bool BeginObject(StringView key) override
        {
            if (m_NodeStack.Empty()) return false;

            const auto& current = CurrentNode();

            // Case A: Sequential iteration over an array (e.g. key == "")
            if (current.is_array())
            {
                usize& currentIndex = m_ArrayIndexStack.Back();

                if (currentIndex >= current.size())
                {
                    VtCoreError(
                        "JsonArchiveReader::BeginObject: Array index out of "
                        "bounds ({}/{})",
                        currentIndex, current.size());
                    return false;
                }

                const auto& element = current[currentIndex++];

                if (!element.is_object())
                {
                    VtCoreError(
                        "JsonArchiveReader::BeginObject: Array element at "
                        "index {} is not an object!",
                        currentIndex - 1);
                    return false;
                }

                m_NodeStack.PushBack(&element);
                m_ArrayIndexStack.PushBack(0);
                return true;
            }

            // Case B: Named lookup in an object
            if (current.is_object())
            {
                std::string keyStr(key.Raw(), key.Size());

                if (!current.contains(keyStr)) return false;

                const auto& child = current[keyStr];
                if (!child.is_object()) return false;

                m_NodeStack.PushBack(&child);
                m_ArrayIndexStack.PushBack(0);
                return true;
            }

            return false;
        }

        void EndObject() override
        {
            if (m_NodeStack.Size() <= 1)
            {
                VtCoreError("JsonArchiveReader::EndObject: Stack underflow!");
                return;
            }

            m_NodeStack.PopBack();
            m_ArrayIndexStack.PopBack();
        }

        bool BeginArray(StringView key, usize& outCount) override
        {
            if (m_NodeStack.Empty()) return false;

            const auto& current = CurrentNode();
            std::string keyStr(key.Raw(), key.Size());

            if (!current.contains(keyStr) || !current[keyStr].is_array())
            {
                outCount = 0;
                return false;
            }

            const auto& arrayNode = current[keyStr];
            outCount              = arrayNode.size();

            m_NodeStack.PushBack(&arrayNode);
            m_ArrayIndexStack.PushBack(0);
            return true;
        }

        bool EndArray() override
        {
            if (m_NodeStack.Size() <= 1)
            {
                VtCoreError("JsonArchiveReader::EndArray: Stack underflow!");
                return false;
            }

            m_NodeStack.PopBack();
            m_ArrayIndexStack.PopBack();
            return true;
        }

        bool BeginArrayElement(usize index)
        {
            const auto& current = CurrentNode();

            if (!current.is_array() || index >= current.size()) return false;

            m_NodeStack.PushBack(&current[index]);
            m_ArrayIndexStack.PushBack(0);
            return true;
        }

        void EndArrayElement()
        {
            if (m_NodeStack.Size() <= 1)
            {
                VtCoreError(
                    "JsonArchiveReader::EndArrayElement: Stack underflow!");
                return;
            }

            m_NodeStack.PopBack();
            m_ArrayIndexStack.PopBack();
        }

        bool Read(StringView key, bool& outValue) override
        {
            return ReadPrimitive(key, outValue);
        }
        bool Read(StringView key, i32& outValue) override
        {
            return ReadPrimitive(key, outValue);
        }
        bool Read(StringView key, i64& outValue) override
        {
            return ReadPrimitive(key, outValue);
        }
        bool Read(StringView key, u32& outValue) override
        {
            return ReadPrimitive(key, outValue);
        }
        bool Read(StringView key, u64& outValue) override
        {
            return ReadPrimitive(key, outValue);
        }
        bool Read(StringView key, f32& outValue) override
        {
            return ReadPrimitive(key, outValue);
        }
        bool Read(StringView key, f64& outValue) override
        {
            return ReadPrimitive(key, outValue);
        }

        bool Read(StringView key, String& outValue) override
        {
            std::string str;
            if (ReadPrimitive(key, str))
            {
                outValue = String(str.data(), str.size());
                return true;
            }
            return false;
        }

        bool Read(StringView key, Path& outValue) override
        {
            std::string str;
            if (ReadPrimitive(key, str))
            {
                outValue = Path(str.c_str());
                return true;
            }
            return false;
        }

      private:
        template <typename T>
        bool ReadPrimitive(StringView key, T& outValue)
        {
            if (m_NodeStack.Empty()) return false;

            const nlohmann::ordered_json& current = *m_NodeStack.Back();
            std::string_view              k(key.Raw(), key.Size());

            if (current.is_object() && current.contains(k))
            {
                try
                {
                    outValue = current[k].get<T>();
                    return true;
                }
                catch (const nlohmann::ordered_json::exception&)
                {
                    return false;
                }
            }
            return false;
        }

        const nlohmann::ordered_json& CurrentNode() const
        {
            return *m_NodeStack.Back();
        }

        nlohmann::ordered_json                m_Root;
        Vector<const nlohmann::ordered_json*> m_NodeStack;
        Vector<usize>                         m_ArrayIndexStack;
    };
    template <CanSerialize T>
    inline static bool SerializeJsonToFile(PathView path, const T& object)
    {
        JsonArchiveWriter writer;
        object.Serialize(writer);

        std::ofstream file(path.Raw());
        if (!file) return false;

        auto str = writer.ToString();

        file << std::string_view(str.Raw(), str.Size());
        return true;
    }
    template <CanDeserialize T>
    inline static bool DeserializeJsonFromFile(PathView path, T& outObject)
    {
        std::ifstream ifs(path.Raw());
        if (!ifs) return false;

        std::stringstream buffer;
        buffer << ifs.rdbuf();
        auto              str = buffer.str();

        JsonArchiveReader reader;
        if (!reader.Parse(StringView(str.data(), str.size()))) return false;

        return outObject.Deserialize(reader);
    }
} // namespace Vortex
