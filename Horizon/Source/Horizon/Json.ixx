/*
 * Created by v1tr10l7 on 21.11.2026.
 * Copyright (c) 2024-2026, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
module;

#include <Prism/Core/Types.hpp>

#include <nlohmann/json.hpp>

#include <fstream>
#include <string>

export module Horizon.Json;

import Horizon.Platform;

/*
 * nlohmann::json speaks std::string; these keep the conversions out of the
 * rest of Horizon.
 */
export namespace Horizon::Json
{
    using Value = nlohmann::json;

    inline String GetString(const Value& object, const char* key, StringView fallback = {})
    {
        if (!object.is_object() || !object.contains(key) || !object[key].is_string())
            return String(fallback);
        return FromStd(object[key].get_ref<const std::string&>());
    }
    inline Path GetPath(const Value& object, const char* key, PathView fallback = {})
    {
        if (!object.is_object() || !object.contains(key) || !object[key].is_string())
            return Path(fallback);
        return Path(StringView(object[key].get_ref<const std::string&>().data(),
                               object[key].get_ref<const std::string&>().size()));
    }
    template <typename T>
    T Get(const Value& object, const char* key, T fallback)
    {
        return object.is_object() ? object.value(key, fallback) : fallback;
    }
    inline const Value& GetArray(const Value& object, const char* key)
    {
        static const Value kEmpty = Value::array();
        if (!object.is_object() || !object.contains(key) || !object[key].is_array())
            return kEmpty;
        return object[key];
    }
    inline const Value& GetObject(const Value& object, const char* key)
    {
        static const Value kEmpty = Value::object();
        if (!object.is_object() || !object.contains(key) || !object[key].is_object())
            return kEmpty;
        return object[key];
    }

    inline Value From(StringView text) { return Value(ToStd(text)); }

    /// Parses `text`; a discarded value (is_discarded()) on bad JSON.
    inline Value Parse(StringView text)
    {
        return Value::parse(text.Raw(), text.Raw() + text.Size(), nullptr, false);
    }
    /// Reads and parses a file; a discarded value if it's missing or bad.
    inline Value ReadFile(PathView path)
    {
        std::ifstream file(ToFs(path));
        if (!file) return Value(Value::value_t::discarded);
        return Value::parse(file, nullptr, false);
    }
    inline String Dump(const Value& value, int indent = 2) { return FromStd(value.dump(indent)); }
} // namespace Horizon::Json
