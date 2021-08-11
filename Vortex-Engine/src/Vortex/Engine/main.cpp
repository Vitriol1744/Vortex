//
// Created by Vitriol1744 on 22.06.2021.
//
#include "Vortex/Engine/Engine.hpp"

int main(int argc, char** argv)
{
    std::vector<char*> args;
    for (int i = 0; i < argc; i++) args.push_back(argv[i]);

    Vortex::Engine* engine = new Vortex::Engine(args);
    engine->Initialize();
    engine->Run();
    engine->Shutdown();

    delete engine;
}

#ifdef _WIN32
#include <Windows.h>
int WINAPI wWinMain(HINSTANCE _In_ hInstance, HINSTANCE _In_opt_ hPrevInstance, LPWSTR _In_ lpCmdLine, int _In_ nCmdShow)
{
    return main(__argc, __argv);
}
#endif

