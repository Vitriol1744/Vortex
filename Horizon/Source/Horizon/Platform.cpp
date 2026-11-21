/*
 * Created by v1tr10l7 on 18.11.2026.
 * Copyright (c) 2024-2026, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
module;

#include <Prism/Core/Types.hpp>

#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <string>

#ifdef _WIN32
    #define WIN32_LEAN_AND_MEAN
    #include <windows.h>
#else
    #include <fcntl.h>
    #include <sys/wait.h>
    #include <unistd.h>
extern char** environ;
#endif

module Horizon.Platform;

import Prism.String.Formatter;

namespace Horizon
{
    namespace
    {
        Path EnvPath(const char* name)
        {
            const char* value = std::getenv(name);
            return value && *value ? Path(value) : Path();
        }
    } // namespace

    Path HomeDirectory()
    {
#ifdef _WIN32
        if (auto home = EnvPath("USERPROFILE"); !home.Empty()) return home;
#else
        if (auto home = EnvPath("HOME"); !home.Empty()) return home;
#endif
        std::error_code ec;
        return FromFs(fs::current_path(ec));
    }

    Path ConfigDirectory()
    {
#ifdef _WIN32
        auto base = EnvPath("APPDATA");
        return (base.Empty() ? HomeDirectory() : base) / "Vortex";
#else
        auto base = EnvPath("XDG_CONFIG_HOME");
        return (base.Empty() ? HomeDirectory() / ".config" : base) / "vortex";
#endif
    }

    Path DataDirectory()
    {
#ifdef _WIN32
        auto base = EnvPath("LOCALAPPDATA");
        return (base.Empty() ? HomeDirectory() : base) / "Vortex";
#else
        auto base = EnvPath("XDG_DATA_HOME");
        return (base.Empty() ? HomeDirectory() / ".local" / "share" : base) / "vortex";
#endif
    }

    Path ExecutableDirectory()
    {
        std::error_code ec;
#ifdef _WIN32
        wchar_t buffer[MAX_PATH];
        DWORD   size = GetModuleFileNameW(nullptr, buffer, MAX_PATH);
        if (size == 0 || size == MAX_PATH) return FromFs(fs::current_path(ec));
        return FromFs(fs::path(buffer).parent_path());
#else
        auto self = fs::read_symlink("/proc/self/exe", ec);
        return FromFs(ec ? fs::current_path(ec) : self.parent_path());
#endif
    }

    String HostPlatform()
    {
#if defined(_WIN32)
        StringView os = "windows";
#elif defined(__APPLE__)
        StringView os = "macos";
#else
        StringView os = "linux";
#endif
#if defined(__x86_64__) || defined(_M_X64)
        StringView arch = "x86_64";
#elif defined(__aarch64__) || defined(_M_ARM64)
        StringView arch = "arm64";
#else
        StringView arch = "unknown";
#endif
        return Format("{}-{}", os, arch);
    }

    String EditorExecutableName()
    {
#ifdef _WIN32
        return "Nexus.exe";
#else
        return "Nexus";
#endif
    }

#ifndef _WIN32
    namespace
    {
        /// argv/envp arrays built before fork(): the child may only make
        /// async-signal-safe calls, so it can't allocate.
        struct ExecArrays
        {
            Vector<String> Storage;
            Vector<char*>  Argv;
            Vector<String> EnvStorage;
            Vector<char*>  Envp;
        };

        void BuildArgv(ExecArrays& arrays, const Vector<String>& argv)
        {
            arrays.Storage = argv;
            for (auto& arg : arrays.Storage) arrays.Argv.PushBack(arg.Raw());
            arrays.Argv.PushBack(nullptr);
        }
        void BuildEnvironment(ExecArrays& arrays, PathView libraryDir)
        {
            constexpr StringView kVar = "LD_LIBRARY_PATH=";
            StringView           previous;
            for (char** env = environ; env && *env; ++env)
            {
                StringView entry = *env;
                if (entry.StartsWith(kVar)) previous = entry.Substr(kVar.Size());
                else arrays.EnvStorage.PushBack(String(entry));
            }
            String value = String(previous);
            if (!libraryDir.Empty())
                value = previous.Empty() ? String(libraryDir.StrView())
                                         : Format("{}:{}", libraryDir.StrView(), previous);
            if (!value.Empty()) arrays.EnvStorage.PushBack(Format("{}{}", kVar, value));

            for (auto& entry : arrays.EnvStorage) arrays.Envp.PushBack(entry.Raw());
            arrays.Envp.PushBack(nullptr);
        }
    } // namespace

    String SpawnDetached(const LaunchRequest& request)
    {
        Vector<String> argv;
        argv.PushBack(String(request.Executable.StrView()));
        for (const auto& arg : request.Arguments) argv.PushBack(arg);

        ExecArrays arrays;
        BuildArgv(arrays, argv);
        BuildEnvironment(arrays, request.LibraryDirectory);

        const String executable = String(request.Executable.StrView());
        const String workingDir = String(request.WorkingDirectory.StrView());
        const String logFile    = String(request.LogFile.StrView());

        // The grandchild reports a failed chdir/exec through this pipe;
        // O_CLOEXEC closes it on a successful exec, so EOF means success.
        int          errorPipe[2];
        if (pipe2(errorPipe, O_CLOEXEC) != 0) return Format("pipe: {}", std::strerror(errno));

        pid_t child = fork();
        if (child < 0)
        {
            close(errorPipe[0]);
            close(errorPipe[1]);
            return Format("fork: {}", std::strerror(errno));
        }
        if (child == 0)
        {
            // Double fork: the grandchild is reparented to init, so the
            // editor never becomes a zombie of the launcher.
            close(errorPipe[0]);
            setsid();
            if (fork() != 0) _exit(0);

            int fail = 0;
            if (!workingDir.Empty() && chdir(workingDir.Raw()) != 0) fail = errno;
            if (!fail && !logFile.Empty())
            {
                int fd = open(logFile.Raw(), O_WRONLY | O_CREAT | O_APPEND | O_CLOEXEC, 0644);
                if (fd >= 0)
                {
                    dup2(fd, STDOUT_FILENO);
                    dup2(fd, STDERR_FILENO);
                }
                int devnull = open("/dev/null", O_RDONLY);
                if (devnull >= 0) dup2(devnull, STDIN_FILENO);
            }
            if (!fail)
            {
                execve(executable.Raw(), arrays.Argv.Raw(), arrays.Envp.Raw());
                fail = errno;
            }
            [[maybe_unused]] auto written = write(errorPipe[1], &fail, sizeof(fail));
            _exit(127);
        }

        close(errorPipe[1]);
        int status = 0;
        waitpid(child, &status, 0);

        int     childErrno = 0;
        ssize_t got        = read(errorPipe[0], &childErrno, sizeof(childErrno));
        close(errorPipe[0]);
        if (got == sizeof(childErrno))
            return Format("Can't start {}: {}", executable, std::strerror(childErrno));
        return String();
    }

    int RunAndWait(const Vector<String>& argv, String* output)
    {
        if (argv.Empty()) return -1;
        ExecArrays arrays;
        BuildArgv(arrays, argv);

        int outPipe[2];
        if (pipe2(outPipe, O_CLOEXEC) != 0) return -1;

        pid_t child = fork();
        if (child < 0)
        {
            close(outPipe[0]);
            close(outPipe[1]);
            return -1;
        }
        if (child == 0)
        {
            dup2(outPipe[1], STDOUT_FILENO);
            dup2(outPipe[1], STDERR_FILENO);
            execvp(arrays.Argv[0], arrays.Argv.Raw());
            _exit(127);
        }
        close(outPipe[1]);

        char    buffer[4096];
        ssize_t count = 0;
        while ((count = read(outPipe[0], buffer, sizeof(buffer))) > 0)
            if (output) *output += StringView(buffer, static_cast<usize>(count));
        close(outPipe[0]);

        int status = 0;
        if (waitpid(child, &status, 0) < 0) return -1;
        return WIFEXITED(status) ? WEXITSTATUS(status) : -1;
    }
#else
    namespace
    {
        std::wstring Wide(StringView text) { return fs::path(ToStd(text)).wstring(); }

        std::wstring QuoteArgument(const std::wstring& arg)
        {
            if (!arg.empty() && arg.find_first_of(L" \t\"") == std::wstring::npos) return arg;
            std::wstring quoted = L"\"";
            for (wchar_t c : arg)
            {
                if (c == L'"') quoted += L'\\';
                quoted += c;
            }
            return quoted + L"\"";
        }
        std::wstring CommandLine(const Vector<String>& argv)
        {
            std::wstring line;
            for (const auto& arg : argv)
            {
                if (!line.empty()) line += L' ';
                line += QuoteArgument(Wide(arg));
            }
            return line;
        }
    } // namespace

    String SpawnDetached(const LaunchRequest& request)
    {
        Vector<String> argv;
        argv.PushBack(String(request.Executable.StrView()));
        for (const auto& arg : request.Arguments) argv.PushBack(arg);
        std::wstring line = CommandLine(argv);

        // Nexus's DLLs are found through PATH, which the child inherits.
        if (!request.LibraryDirectory.Empty())
        {
            std::wstring path = Wide(request.LibraryDirectory.StrView());
            if (const wchar_t* old = _wgetenv(L"PATH")) path += L";" + std::wstring(old);
            SetEnvironmentVariableW(L"PATH", path.c_str());
        }

        STARTUPINFOW        startup{.cb = sizeof(STARTUPINFOW)};
        PROCESS_INFORMATION info{};
        std::wstring        cwd = Wide(request.WorkingDirectory.StrView());
        BOOL ok = CreateProcessW(nullptr, line.data(), nullptr, nullptr, FALSE,
                                 DETACHED_PROCESS | CREATE_NEW_PROCESS_GROUP, nullptr,
                                 cwd.empty() ? nullptr : cwd.c_str(), &startup, &info);
        if (!ok)
            return Format("Can't start {} (error {})", request.Executable,
                          static_cast<u32>(GetLastError()));
        CloseHandle(info.hThread);
        CloseHandle(info.hProcess);
        return String();
    }

    int RunAndWait(const Vector<String>& argv, String* output)
    {
        (void)output;
        std::wstring        line = CommandLine(argv);
        STARTUPINFOW        startup{.cb = sizeof(STARTUPINFOW)};
        PROCESS_INFORMATION info{};
        if (!CreateProcessW(nullptr, line.data(), nullptr, nullptr, FALSE, CREATE_NO_WINDOW,
                            nullptr, nullptr, &startup, &info))
            return -1;
        WaitForSingleObject(info.hProcess, INFINITE);
        DWORD code = 0;
        GetExitCodeProcess(info.hProcess, &code);
        CloseHandle(info.hThread);
        CloseHandle(info.hProcess);
        return static_cast<int>(code);
    }
#endif
} // namespace Horizon
