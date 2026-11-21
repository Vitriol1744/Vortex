/*
 * Created by v1tr10l7 on 18.11.2026.
 * Copyright (c) 2024-2026, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
module;

#include <Prism/Core/Types.hpp>

#include <curl/curl.h>

#include <cstdio>
#include <filesystem>
#include <mutex>

export module Horizon.Http;

export import Prism.Utility.Atomic;
export import Prism.Utility.Delegate;
import Prism.String.Formatter;
import Horizon.Platform;

export namespace Horizon::Http
{
    struct Result
    {
        bool   Ok     = false;
        long   Status = 0;
        String Error;
    };

    /// (bytes received, total bytes or 0 when the server didn't say).
    using ProgressCallback = Delegate<void(u64, u64)>;

    /// GET `url` into `body`. http(s):// and file:// URLs both work.
    Result Get(const String& url, String& body);

    /// GET `url` into `destination`, reporting progress. Setting `*cancel`
    /// aborts the transfer. A partial file is removed on failure.
    Result Download(const String& url, PathView destination,
                    const ProgressCallback& progress = {},
                    const Atomic<bool>*     cancel   = nullptr);
} // namespace Horizon::Http

namespace Horizon::Http
{
    namespace
    {
        void GlobalInit()
        {
            static std::once_flag once;
            std::call_once(once, [] { curl_global_init(CURL_GLOBAL_DEFAULT); });
        }

        CURL* NewHandle(const String& url, char* errorBuffer)
        {
            GlobalInit();
            CURL* curl = curl_easy_init();
            if (!curl) return nullptr;
            curl_easy_setopt(curl, CURLOPT_URL, url.Raw());
            curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
            curl_easy_setopt(curl, CURLOPT_FAILONERROR, 1L);
            curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1L);
            curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 15L);
            // Give up on a stalled transfer (< 1 B/s for 60 s), not a slow one.
            curl_easy_setopt(curl, CURLOPT_LOW_SPEED_LIMIT, 1L);
            curl_easy_setopt(curl, CURLOPT_LOW_SPEED_TIME, 60L);
            curl_easy_setopt(curl, CURLOPT_USERAGENT, "Vortex-Horizon/0.1");
            curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, errorBuffer);
            errorBuffer[0] = '\0';
            return curl;
        }

        Result Perform(CURL* curl, const char* errorBuffer)
        {
            Result   result;
            CURLcode code = curl_easy_perform(curl);
            curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &result.Status);
            result.Ok = code == CURLE_OK;
            if (!result.Ok)
                result.Error = errorBuffer[0] ? errorBuffer : curl_easy_strerror(code);
            curl_easy_cleanup(curl);
            return result;
        }

        size_t AppendToString(char* data, size_t size, size_t count, void* user)
        {
            *static_cast<String*>(user) += StringView(data, size * count);
            return size * count;
        }
        size_t WriteToFile(char* data, size_t size, size_t count, void* user)
        {
            return std::fwrite(data, size, count, static_cast<std::FILE*>(user)) * size;
        }

        struct ProgressContext
        {
            const ProgressCallback* Callback;
            const Atomic<bool>*     Cancel;
        };
        int OnProgress(void* user, curl_off_t total, curl_off_t now, curl_off_t,
                       curl_off_t)
        {
            auto& context = *static_cast<ProgressContext*>(user);
            if (context.Cancel && context.Cancel->Load()) return 1;
            if (context.Callback && *context.Callback)
                (*context.Callback)(static_cast<u64>(now), static_cast<u64>(total));
            return 0;
        }
    } // namespace

    Result Get(const String& url, String& body)
    {
        char  errorBuffer[CURL_ERROR_SIZE];
        CURL* curl = NewHandle(url, errorBuffer);
        if (!curl) return {.Error = "curl_easy_init failed"};
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, AppendToString);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &body);
        return Perform(curl, errorBuffer);
    }

    Result Download(const String& url, PathView destination, const ProgressCallback& progress,
                    const Atomic<bool>* cancel)
    {
        const String target = String(destination.StrView());
        std::FILE*   file   = std::fopen(target.Raw(), "wb");
        if (!file) return {.Error = Format("Can't write {}", target)};

        char  errorBuffer[CURL_ERROR_SIZE];
        CURL* curl = NewHandle(url, errorBuffer);
        if (!curl)
        {
            std::fclose(file);
            return {.Error = "curl_easy_init failed"};
        }

        ProgressContext context{&progress, cancel};
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteToFile);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, file);
        curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0L);
        curl_easy_setopt(curl, CURLOPT_XFERINFOFUNCTION, OnProgress);
        curl_easy_setopt(curl, CURLOPT_XFERINFODATA, &context);

        Result result = Perform(curl, errorBuffer);
        if (std::fclose(file) != 0 && result.Ok)
            result = {.Error = Format("Can't finish writing {}", target)};
        if (cancel && cancel->Load()) result = {.Error = "Cancelled"};

        if (!result.Ok)
        {
            std::error_code ec;
            fs::remove(ToFs(destination), ec);
        }
        return result;
    }
} // namespace Horizon::Http
