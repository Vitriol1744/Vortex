/*
 * Created by v1tr10l7 on 06.09.2026.
 * Copyright (c) 2024-2026, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

import Prism.Utility.Path;
#include <Vortex/Core/Core.hpp>
#include <Vortex/Core/Math/Vector.hpp>
#include <Vortex/Project/Export.hpp>
#include <Vortex/Serialization/Archive.hpp>

namespace Vortex
{
    inline void SerializeValue(ArchiveWriter& writer, StringView key,
                               const Math::Vec2u& vec)
    {
        writer.BeginObject(key);
        writer.Write("x", vec.x);
        writer.Write("y", vec.y);
        writer.EndObject();
    }
    inline bool DeserializeValue(ArchiveReader& reader, StringView key,
                                 Math::Vec2u& vec)
    {
        usize count = 0;
        if (!reader.BeginObject(key)) return false;
        if (count != 2)
        {
            reader.EndObject();
            return false;
        }

        reader.Read("x", vec.x);
        reader.Read("y", vec.y);

        reader.EndObject();
        return true;
    }

    struct VT_PROJECT_API ProjectMetadata
    {
        String  Name        = "Untitled";
        String  Author      = "Unknown";
        String  Description = "No description provided.";
        Version Version     = {0, 1, 0};

        void    Serialize(ArchiveWriter& archive) const;
        bool    Deserialize(ArchiveReader& archive);
    };
    struct VT_PROJECT_API RendererSettings
    {
        usize       TargetFrameRate     = 60;
        bool        VSync               = true;
        usize       AntiAliasingSamples = 0;
        Math::Vec2u Resolution          = {1280, 720};

        void        Serialize(ArchiveWriter& archive) const;
        bool        Deserialize(ArchiveReader& archive);
    };

    struct VT_PROJECT_API ProjectConfig
    {
        ProjectMetadata  Metadata;
        RendererSettings RendererSettings;

        Path             StartScenePath;
        Path             AssetDirectory   = "Assets";
        Path             ScriptModulePath = "Binaries/Script.dll";

        void             Serialize(ArchiveWriter& archive) const;
        bool             Deserialize(ArchiveReader& archive);
    };
    class VT_PROJECT_API Project : public RefCounted<Project>
    {
      public:
        Project() = default;

        static ::Ref<Project> Load(PathView path);

        inline PathView       Path() const { return m_ProjectDirectory; }
        ProjectConfig&        Config() { return m_Config; }
        const ProjectConfig&  Config() const { return m_Config; }

        const ::Path& ProjectDirectory() const { return m_ProjectDirectory; }

        inline ::Path AssetDirectory() const
        {
            return m_ProjectDirectory / m_Config.AssetDirectory;
        }

        inline ::Path AssetAbsolute(const ::Path& relativePath) const
        {
            return AssetDirectory() / relativePath;
        }

        static ::Ref<Project> Active() { return s_ActiveProject; }
        static void           SetActive(::Ref<Project> project)
        {
            s_ActiveProject = project;
        }

      private:
        ::Path                       m_ProjectDirectory;
        ProjectConfig                m_Config;

        inline static ::Ref<Project> s_ActiveProject = nullptr;

        friend class ::Ref<Project>;
        template <typename T, typename... Args>
        friend constexpr ::Ref<T> Prism::CreateRef(Args&&... args);

        Project(PathView projectDirectory, const ProjectConfig& config)
            : m_ProjectDirectory(projectDirectory)
            , m_Config(config)
        {
        }
    };
}; // namespace Vortex
