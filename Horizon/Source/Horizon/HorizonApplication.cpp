/*
 * Created by v1tr10l7 on 18.11.2026.
 * Copyright (c) 2024-2026, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#include <Vortex/Engine/Application.hpp>

#include <Horizon/LauncherLayer.hpp>

#include <filesystem>

import Horizon.Cli;
import Horizon.Platform;

using namespace Vortex;

namespace Horizon
{
    class HorizonApplication final : public Application
    {
      public:
        explicit HorizonApplication(const ApplicationSpecification& spec)
            : Application(spec)
        {
            PushLayer(CreateRef<LauncherLayer>());
        }
    };

    namespace
    {
        /// The ImGui layer loads assets/fonts relative to the working
        /// directory. Started from a desktop entry or a file manager, the
        /// cwd is arbitrary, so look next to the executable (installed
        /// layout) and one level up (bin/ in a dev tree).
        void EnterAssetDirectory()
        {
            if (Exists("assets/fonts")) return;

            Path exe = ExecutableDirectory();
            for (const Path& candidate : {exe, exe.ParentPath()})
            {
                if (!Exists(candidate / "assets" / "fonts")) continue;
                std::error_code ec;
                std::filesystem::current_path(ToFs(candidate), ec);
                return;
            }
        }
    } // namespace
} // namespace Horizon

ApplicationLaunch Vortex::CreateApplication(const CommandLineArguments& args)
{
    // Headless commands (--install, --list-releases...) never open a window.
    Vector<String> argv;
    for (usize i = 1; i < args.Count(); ++i) argv.PushBack(String(args[i]));
    if (auto code = Horizon::RunCommandLine(argv)) return ApplicationLaunch::Exit(*code);

    Horizon::EnterAssetDirectory();

    ApplicationSpecification spec{};
    spec.Name         = "Horizon";
    spec.Arguments    = args;
    spec.WindowWidth  = 1180;
    spec.WindowHeight = 760;
    // A launcher only needs a window and ImGui: no .NET, PhysX or scene
    // renderer.
    spec.Subsystems   = {
          .SceneRenderer = false,
          .Physics       = false,
          .Scripting     = false,
    };
    return CreateScope<Horizon::HorizonApplication>(spec);
}
