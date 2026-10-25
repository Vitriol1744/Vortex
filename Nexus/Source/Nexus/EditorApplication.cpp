/*
 * Created by v1tr10l7 on 07.09.2024.
 * Copyright (c) 2024-2024, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#include <Vortex/Core/Log/Log.hpp>
#include <Vortex/Engine/Application.hpp>

#include <Nexus/EditorLayer.hpp>

using namespace Vortex;

namespace Vortex::Editor
{
    class EditorApplication : public Application
    {
      public:
        EditorApplication(const ApplicationSpecification& spec)
            : Application(spec)
        {
            PushLayer(CreateRef<Editor::EditorLayer>());
        }
    };

    ::Scope<ConsoleSink> g_ConsoleSink = nullptr;
} // namespace Vortex::Editor

Application* __attribute__((visibility("default")))
Vortex::CreateApplication(const CommandLineArguments&)
{
    ApplicationSpecification spec{};
    Editor::g_ConsoleSink = CreateScope<Editor::ConsoleSink>(nullptr);
    Log::CoreLogger().AddSink(Editor::g_ConsoleSink.Raw());
    Log::ClientLogger().AddSink(Editor::g_ConsoleSink.Raw());

    return new Editor::EditorApplication(spec);
}
