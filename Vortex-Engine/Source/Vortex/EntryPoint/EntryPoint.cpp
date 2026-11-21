/*
 * Created by v1tr10l7 on 17.07.2024.
 * Copyright (c) 2024-2024, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#include <Vortex/Core/Core.hpp>
#include <Vortex/Engine/Application.hpp>

#include <cstdlib>

namespace Vortex
{
    /**
     * The process's main(): asks the program for its application and runs
     * it, again for as long as it asks to restart (e.g. Nexus after a GPU
     * change). CreateApplication may instead return an exit code, for
     * commands that finish without a window.
     */
    int Main(int argc, char** argv)
    {
        CommandLineArguments args(argc, argv);
        Log::Initialize();

        int exitCode = EXIT_SUCCESS;
        for (;;)
        {
            ApplicationLaunch launch = CreateApplication(args);
            if (!launch.App)
            {
                exitCode = launch.ExitCode;
                break;
            }
            bool restart = launch.App->Run();
            // Destroyed before a restart creates the next one: only one
            // Application may exist at a time.
            launch.App.Reset();
            if (!restart) break;
        }

        Log::Shutdown();
        return exitCode;
    }
} // namespace Vortex

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

// TODO(v1tr10l7): on Android libVortex-Engine is the whole app, so this
// placeholder stands in for the game; android_main doesn't run it yet.
using namespace Vortex;
class Game : public Application
{
  public:
    Game(const ApplicationSpecification& spec)
        : Application(spec)
    {
    }
};

ApplicationLaunch Vortex::CreateApplication(const CommandLineArguments&)
{
    ApplicationSpecification spec{};
    return CreateScope<Game>(spec);
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

/// Read by LeakSanitizer when the build is sanitized (unused otherwise).
/// Third-party leaks LeakSanitizer can't be told about otherwise:
/// - Coral converts the managed-allocated Coral::String returned by
///   Type::GetFullName() into std::string keys (TypeCache, LoadAssembly)
///   without freeing it; one allocation per type, once at startup.
/// - The .NET runtime keeps its own allocations until process exit.
extern "C" __attribute__((visibility("default"), used)) const char*
__lsan_default_suppressions()
{
    return "leak:libcoreclr.so\n"
           "leak:libclrjit.so\n"
           "leak:libhostfxr.so\n"
           "leak:libhostpolicy.so\n"
           "leak:Coral::\n";
}
