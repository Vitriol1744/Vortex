//
// Created by Vitriol1744 on 22.06.2021.
//
#pragma once

int main(int argc, char** argv)
{
    Vortex::Application* app = Vortex::CreateApplication();
    app->Run();
}

#ifdef _WIN32
/*#include <Windows.h>
int WINAPI wWinMain(HINSTANCE _In_ hInstance, HINSTANCE _In_opt_ hPrevInstance, LPWSTR _In_ lpCmdLine, int _In_ nCmdShow)
{
    return main(0, nullptr);
}*/
#endif

