/*
 * Created by v1tr10l7 on 14.09.2026.
 * Copyright (c) 2024-2026, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

// System headers first: they must precede any `import` in this TU.
#include <fcntl.h>
#include <spawn.h>
#include <sys/wait.h>
#include <unistd.h>

#include <Vortex/Core/Process.hpp>

import Prism.Containers.Array;

namespace Vortex
{
    class PosixProcess final : public Process
    {
      public:
        PosixProcess(String command, Vector<String> args)
            : m_Command(Move(command))
            , m_Args(Move(args))
        {
        }

        ~PosixProcess() override
        {
            Kill();
            Cleanup();
        }

        bool Start() override
        {
            if (m_State == ProcessState::eRunning) return false;

            if (pipe(m_StdOutPipe) < 0 || pipe(m_StdErrPipe) < 0)
            {
                m_State = ProcessState::eFailed;
                return false;
            }

            // Enable non-blocking I/O on output read handles
            fcntl(m_StdOutPipe[0], F_SETFL, O_NONBLOCK);
            fcntl(m_StdErrPipe[0], F_SETFL, O_NONBLOCK);

            posix_spawn_file_actions_t actions;
            posix_spawn_file_actions_init(&actions);

            // Redirect standard descriptors in child process
            posix_spawn_file_actions_adddup2(&actions, m_StdOutPipe[1],
                                             STDOUT_FILENO);
            posix_spawn_file_actions_adddup2(&actions, m_StdErrPipe[1],
                                             STDERR_FILENO);
            posix_spawn_file_actions_addclose(&actions, m_StdOutPipe[0]);
            posix_spawn_file_actions_addclose(&actions, m_StdErrPipe[0]);

            Vector<char*> argv;
            argv.PushBack(const_cast<char*>(m_Command.Raw()));
            for (const auto& arg : m_Args)
                argv.PushBack(const_cast<char*>(arg.Raw()));
            argv.PushBack(nullptr);

            i32 status = posix_spawnp(&m_ID, m_Command.Raw(), &actions, nullptr,
                                      argv.Raw(), environ);
            posix_spawn_file_actions_destroy(&actions);

            // Close unused child write pipes in host process
            close(m_StdOutPipe[1]);
            close(m_StdErrPipe[1]);
            m_StdOutPipe[1] = -1;
            m_StdErrPipe[1] = -1;

            if (status != 0)
            {
                m_State = ProcessState::eFailed;
                return false;
            }

            m_State = ProcessState::eRunning;
            return true;
        }

        ProcessState Poll() override
        {
            if (m_State != ProcessState::eRunning) return m_State;

            ReadPipes();

            i32   status = 0;
            pid_t res    = waitpid(m_ID, &status, WNOHANG);

            if (res == m_ID)
            {
                if (WIFEXITED(status)) m_ExitCode = WEXITSTATUS(status);
                else m_ExitCode = -1;

                m_State = ProcessState::eExited;
            }
            return m_State;
        }

        ProcessResult Wait() override
        {
            while (Poll() == ProcessState::eRunning) { usleep(1000); }
            return ProcessResult{.exitCode     = m_ExitCode,
                                 .stdoutOutput = Move(m_StdOutBuffer),
                                 .stderrOutput = Move(m_StdErrBuffer)};
        }

        void Kill() override
        {
            if (m_State == ProcessState::eRunning && m_ID > 0)
            {
                kill(m_ID, SIGKILL);
                m_State    = ProcessState::eExited;
                m_ExitCode = -1;
            }
        }
        ProcessState State() const override { return m_State; }

      private:
        String         m_Command;
        Vector<String> m_Args;
        ProcessState   m_State{ProcessState::eReady};
        pid_t          m_ID{-1};
        i32            m_ExitCode{-1};

        String         m_StdOutBuffer;
        String         m_StdErrBuffer;

        i32            m_StdOutPipe[2]{-1, -1};
        i32            m_StdErrPipe[2]{-1, -1};

        void           ReadPipes()
        {
            auto readFromFd = [](i32 fd, String& buffer)
            {
                Array<char, 512> chunk;
                isize            bytesRead = 0;
                while ((bytesRead = read(fd, chunk.Raw(), chunk.Size())) > 0)
                    buffer += StringView(chunk.Raw(),
                                         static_cast<usize>(bytesRead));
            };

            if (m_StdOutPipe[0] != -1)
                readFromFd(m_StdOutPipe[0], m_StdOutBuffer);
            if (m_StdErrPipe[0] != -1)
                readFromFd(m_StdErrPipe[0], m_StdErrBuffer);
        }

        void Cleanup()
        {
            if (m_StdOutPipe[0] != -1) close(m_StdOutPipe[0]);
            if (m_StdErrPipe[0] != -1) close(m_StdErrPipe[0]);
            m_StdOutPipe[0] = -1;
            m_StdErrPipe[0] = -1;
        }
    };
}; // namespace Vortex
