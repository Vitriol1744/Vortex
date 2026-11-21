/*
 * Created by v1tr10l7 on 13.09.2026.
 * Copyright (c) 2024-2026, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

import Prism.Utility.Path;
#include <Vortex/Serialization/Stream.hpp>

#include <fstream>

namespace Vortex
{
    class FileStreamReader : public StreamReader
    {
      public:
        explicit FileStreamReader(PathView path)
            : m_Path(path)
            , m_Stream(path.Raw(), std::ios::in | std::ios::binary)
        {
        }

        virtual ~FileStreamReader() override
        {
            if (m_Stream.is_open()) m_Stream.close();
        }

        FileStreamReader(const FileStreamReader&)                = delete;
        FileStreamReader& operator=(const FileStreamReader&)     = delete;

        FileStreamReader(FileStreamReader&&) noexcept            = default;
        FileStreamReader& operator=(FileStreamReader&&) noexcept = default;

        virtual bool      Good() const override { return m_Stream.good(); }

        virtual u64       Position() const override
        {
            return static_cast<u64>(
                const_cast<std::ifstream&>(m_Stream).tellg());
        }

        virtual bool SetPosition(u64 position) override
        {
            m_Stream.seekg(static_cast<std::streamoff>(position));
            return m_Stream.good();
        }

        virtual bool ReadBytes(char* destination, usize size) override
        {
            m_Stream.read(destination, static_cast<std::streamsize>(size));
            return m_Stream.good();
        }

        PathView Path() const { return m_Path; }

      private:
        class Path    m_Path;
        std::ifstream m_Stream;
    };

    class FileStreamWriter : public StreamWriter
    {
      public:
        explicit FileStreamWriter(PathView path)
            : m_Path(path)
            , m_Stream(path.Raw(),
                       std::ios::out | std::ios::binary | std::ios::trunc)
        {
        }

        virtual ~FileStreamWriter() override
        {
            if (m_Stream.is_open()) m_Stream.close();
        }

        FileStreamWriter(const FileStreamWriter&)                = delete;
        FileStreamWriter& operator=(const FileStreamWriter&)     = delete;

        FileStreamWriter(FileStreamWriter&&) noexcept            = default;
        FileStreamWriter& operator=(FileStreamWriter&&) noexcept = default;

        virtual bool      Good() const override { return m_Stream.good(); }

        virtual u64       Position() const override
        {
            return static_cast<u64>(
                const_cast<std::ofstream&>(m_Stream).tellp());
        }
        virtual bool SetPosition(u64 position) override
        {
            m_Stream.seekp(static_cast<std::streamoff>(position));
            return m_Stream.good();
        }

        virtual bool WriteBytes(const char* data, usize size) override
        {
            m_Stream.write(data, static_cast<std::streamsize>(size));
            return m_Stream.good();
        }

        inline PathView Path() const { return m_Path; }

      private:
        class Path    m_Path;
        std::ofstream m_Stream;
    };
} // namespace Vortex
