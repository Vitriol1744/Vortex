/*
 * Created by v1tr10l7 on 17.07.2024.
 * Copyright (c) 2024-2024, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#include <Prism/Containers/Array.hpp>
#include <Vortex/Core/Core.hpp>
#include <Vortex/Engine/Application.hpp>

#define VT_TEST_ANDROID_BUILD 0
namespace Vortex
{
    void PrintBranding();

    int  Main(int argc, char** argv)
    {
        PrintBranding();
        (void)argc;
        (void)argv;
        CommandLineArguments args;
        Log::Initialize();

        VtCoreTrace("Launching the {}...", Vortex::g_EngineName);
        bool shouldRestart = false;
        do
        {
            Application* app = CreateApplication(args);

            shouldRestart    = app->Run();

            delete app;
        } while (shouldRestart);

        Log::Shutdown();
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
#elifdef VT_PLATFORM_ANDROID
    #include <Vortex/Core/Log/Log.hpp>
    #include <android/log.h>
    #include <android/native_activity.h>
    #include <android/versioning.h>
    #include <android/window.h>
    #include <android_native_app_glue.h>
    #include <jni.h>

static android_app* s_App = nullptr;

extern "C" [[maybe_unused]]
JNIEXPORT void JNICALL Java_com_v1tr10l7_vortex_Activity_nativeSetPlatformInfo(
    JNIEnv* env, jobject thiz, jstring appPackageName,
    jstring deviceManufacturer, jstring deviceModel, jstring deviceBuildNumber,
    jstring systemVersion, jstring systemLanguage, i32 screenWidth,
    i32 screenHeight, jstring cacheDir, jstring executablePath)
{
    VtCoreInfo("nativeSetPlatformInfo");
}

using namespace Vortex;
class Game : public Application
{
  public:
    Game(const ApplicationSpecification& spec)
        : Application(spec)
    {
    }
};

Application* Vortex::CreateApplication(const CommandLineArguments&)
{
    ApplicationSpecification spec{};
    return new Game(spec);
}

void android_main(android_app* app)
{
    Vortex::Log::Initialize();
    VtCoreInfo("AndroidMain:");
    s_App = app;
    // app->onAppCmd     = OnAppCmd;
    // app->onInputEvent = OnAppInput;
    ANativeActivity_setWindowFlags(
        app->activity,
        AWINDOW_FLAG_KEEP_SCREEN_ON | AWINDOW_FLAG_TURN_SCREEN_ON
            | AWINDOW_FLAG_FULLSCREEN | AWINDOW_FLAG_DISMISS_KEYGUARD,
        0);
    ANativeActivity_setWindowFormat(app->activity, WINDOW_FORMAT_RGBA_8888);
    pthread_setname_np(pthread_self(), "Main");
}
#endif
