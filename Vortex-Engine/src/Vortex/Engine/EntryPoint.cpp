//
// Created by Vitriol1744 on 05.09.2022.
//
#include "Vortex/vtpch.hpp"
#include "Engine.hpp"

int main(int argc, char** argv)
{
    std::vector<char*> args;
    for (Vortex::int32 i = 0; i < argc; i++) args.push_back(argv[i]);

    auto engine = new Vortex::Engine(args);
    if (!engine->Initialize()) return EXIT_FAILURE;
    engine->Run();
    engine->Shutdown();

    delete engine;
    return EXIT_SUCCESS;
}

#ifdef VT_PLATFORM_WINDOWS
#include <windows.h>
int WINAPI wWinMain(HINSTANCE _In_ hInstance, HINSTANCE _In_opt_ hPrevInstance, LPWSTR _In_ lpCmdLine, int _In_ nCmdShow)
{
    return main(__argc, __argv);
}
#endif
