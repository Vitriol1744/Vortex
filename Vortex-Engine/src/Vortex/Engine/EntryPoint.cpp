/*
 * Created by v1tr10l7 on 17.07.2024.
 * Copyright (c) 2024-2024, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#include "vtpch.hpp"

#include "Vortex/Core/Core.hpp"
#include "Vortex/Engine/Application.hpp"

namespace Vortex
{
    void PrintBranding();

    int  Main(int argc, char** argv)
    {
        PrintBranding();
        (void)argc;
        (void)argv;
        CommandLineArguments args;

        VtCoreTrace("Launching the {}...", Vortex::g_EngineName);
        bool shouldRestart = false;
        do {
            Application* app = CreateApplication(args);

            shouldRestart    = app->Run();

            delete app;
        } while (shouldRestart);

        return EXIT_SUCCESS;
    }

    void PrintBranding()
    {
        printf(
            " __      __        _              ______             _            "
            "\n");
        printf(
            " \\ \\    / /       | |            |  ____|           (_)         "
            "  \n");
        printf(
            "  \\ \\  / /__  _ __| |_ _____  __ | |__   _ __   __ _ _ _ __   "
            "___ \n");
        printf(
            "   \\ \\/ / _ \\| '__| __/ _ \\ \\/ / |  __| | '_ \\ / _` | | '_ "
            "\\ / _ \\\n");
        printf(
            "    \\  / (_) | |  | ||  __/>  <  | |____| | | | (_| | | | | |  "
            "__/\n");
        printf(
            "     \\/ \\___/|_|   \\__\\___/_/\\_\\ |______|_| |_|\\__, |_|_| "
            "|_|\\___|\n");
        printf(
            "                                                __/ |             "
            "\n");
        printf(
            "                                               |___/              "
            "\n");
    }
}; // namespace Vortex

int main(int argc, char** argv) { return Vortex::Main(argc, argv); }

#ifdef VT_PLATFORM_WINDOWS
    #include <Windows.h>

int WINAPI wWinMain(HINSTANCE _In_ hInstance, HINSTANCE _In_opt_ hPrevInstance,
                    LPWSTR _In_ lpCmdLine, int _In_ nCmdShow)
{
    (void)hInstance;
    (void)hPrevInstance;
    (void)lpCmdLine;
    (void)nCmdShow;
    return Vortex::Main(__argc, __argv);
}
#endif
