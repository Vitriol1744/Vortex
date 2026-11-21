/*
 * Created by v1tr10l7 on 07.09.2024.
 * Copyright (c) 2024-2026, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#include <Vortex/Core/Log/Log.hpp>
#include <Vortex/Engine/Application.hpp>

#include <Nexus/EditorLayer.hpp>
#include <Nexus/EditorPreferences.hpp>
#include <Nexus/UI/Windows/ConsoleWindow.hpp>

#include <Vortex/Serialization/JsonArchive.hpp>

using namespace Vortex;

namespace Vortex::Editor
{
    class EditorApplication final : public Application
    {
      public:
        explicit EditorApplication(const ApplicationSpecification& spec)
            : Application(spec)
        {
            PushLayer(CreateRef<EditorLayer>());
        }
        ~EditorApplication() override
        {
            // The sink outlives the application (loggers use it until
            // Log::Shutdown); don't let it keep the console window alive.
            if (auto& sink = ConsoleSink::Instance()) sink->SetTarget(nullptr);
        }
    };
} // namespace Vortex::Editor

ApplicationLaunch Vortex::CreateApplication(const CommandLineArguments& args)
{
    // Installed before anything logs, so the console shows startup too.
    auto& sink = Editor::ConsoleSink::Instance();
    sink       = CreateRef<Editor::ConsoleSink>();
    Log::CoreLogger().AddSink(sink.Raw());
    Log::ClientLogger().AddSink(sink.Raw());

    ApplicationSpecification spec{};
    spec.Name      = "Nexus";
    spec.Arguments = args;

    // The GPU is picked when the renderer starts, so it comes from the
    // preferences file now (the settings change it, then restart).
    Editor::EditorPreferences preferences;
    if (DeserializeJsonFromFile(PathView(Editor::EditorPreferences::FileName), preferences))
    {
        spec.PreferredAdapter = preferences.PreferredGpu;
        spec.VSync            = preferences.VSync;
        spec.RendererAPI      = preferences.RendererAPI;
    }
    return CreateScope<Editor::EditorApplication>(spec);
}
