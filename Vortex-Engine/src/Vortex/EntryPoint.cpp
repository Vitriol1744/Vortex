//
// Created by Vitriol1744 on 05.09.2022.
//
#include "Vortex/Engine.hpp"

#include <vector>

int main(int argc, char** argv)
{
    std::vector<char*> args;
    for (Vortex::uint32 i = 0; i < argc; i++) args.push_back(argv[i]);

    auto engine = new Vortex::Engine(args);
    engine->Initialize();
    engine->Run();
    engine->Shutdown();

    delete engine;
}

#ifdef VT_PLATFORM_WINDOWS
#include <Windows.h>
int WINAPI wWinMain(HINSTANCE _In_ hInstance, HINSTANCE _In_opt_ hPrevInstance, LPWSTR _In_ lpCmdLine, int _In_ nCmdShow)
{
    return main(__argc, __argv);
}
#endif
