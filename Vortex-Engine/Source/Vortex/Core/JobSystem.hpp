/*
 * Created by v1tr10l7 on 29.09.2026.
 * Copyright (c) 2024-2026, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include <Prism/Containers/Deque.hpp>
#include <Prism/Containers/Vector.hpp>
#include <Prism/Memory/Scope.hpp>
#include <Prism/Utility/Atomic.hpp>
#include <Prism/Utility/Delegate.hpp>

#include <condition_variable>
#include <mutex>
#include <thread>

namespace Vortex
{
    class JobSystem
    {
      public:
        using Job = Delegate<void()>;

      private:
        struct WorkerQueue
        {
            Prism::Deque<Job> Jobs;
            std::mutex        Mutex;
        };

        Vector<std::thread>        m_Workers;
        Vector<Scope<WorkerQueue>> m_Queues;
        Atomic<bool>               m_Running{true};
        Atomic<u32>                m_ActiveJobs{0};
        std::condition_variable    m_WakeCondition;
        std::mutex                 m_WakeMutex;

        static thread_local i32    s_WorkerIndex;

      public:
        explicit JobSystem(u32 threadCount
                           = std::thread::hardware_concurrency())
        {
            if (threadCount == 0) threadCount = 1;

            m_Queues.Reserve(threadCount);
            for (u32 i = 0; i < threadCount; ++i)
                m_Queues.PushBack(CreateScope<WorkerQueue>());

            for (u32 i = 0; i < threadCount; ++i)
            {
                m_Workers.EmplaceBack(
                    [this, i]()
                    {
                        s_WorkerIndex = static_cast<i32>(i);
                        WorkerLoop(i);
                    });
            }
        }

        ~JobSystem()
        {
            m_Running.Store(false, MemoryOrder::eRelaxed);
            m_WakeCondition.notify_all();

            for (auto& thread : m_Workers)
                if (thread.joinable()) thread.join();
        }

        void Execute(Job job)
        {
            m_ActiveJobs.FetchAdd(1, MemoryOrder::eRelaxed);

            int target_queue = s_WorkerIndex;
            if (target_queue < 0) target_queue = 0;

            {
                std::lock_guard<std::mutex> lock(m_Queues[target_queue]->Mutex);
                m_Queues[target_queue]->Jobs.PushBack(Move(job));
            }

            m_WakeCondition.notify_one();
        }

        void WaitIdle()
        {
            while (m_ActiveJobs.Load(MemoryOrder::eRelaxed) > 0)
            {
                Job job;
                if (PopOrStealJob(s_WorkerIndex >= 0 ? s_WorkerIndex : 0, job))
                {
                    job();
                    m_ActiveJobs.FetchSub(1, MemoryOrder::eRelaxed);
                }
                else std::this_thread::yield();
            }
        }

      private:
        void WorkerLoop(u32 workerIndex)
        {
            while (m_Running.Load(MemoryOrder::eRelaxed))
            {
                Job job;
                if (PopOrStealJob(workerIndex, job))
                {
                    job();
                    m_ActiveJobs.FetchSub(1, MemoryOrder::eRelaxed);
                }
                else {
                    std::unique_lock<std::mutex> lock(m_WakeMutex);
                    m_WakeCondition.wait_for(
                        lock, std::chrono::milliseconds(1),
                        [this]()
                        {
                            return !m_Running.Load(MemoryOrder::eRelaxed)
                                || m_ActiveJobs.Load(MemoryOrder::eRelaxed) > 0;
                        });
                }
            }
        }

        bool PopOrStealJob(u32 workerIndex, Job& outJob)
        {
            {
                std::lock_guard<std::mutex> lock(m_Queues[workerIndex]->Mutex);
                if (!m_Queues[workerIndex]->Jobs.Empty())
                {
                    outJob = Move(m_Queues[workerIndex]->Jobs.Back());
                    m_Queues[workerIndex]->Jobs.PopBack();
                    return true;
                }
            }

            u32 queueCount = static_cast<u32>(m_Queues.Size());
            for (u32 i = 1; i < queueCount; ++i)
            {
                u32 stealIndex = (workerIndex + i) % queueCount;
                std::lock_guard<std::mutex> lock(m_Queues[stealIndex]->Mutex);
                if (!m_Queues[stealIndex]->Jobs.Empty())
                {
                    outJob = Move(m_Queues[stealIndex]->Jobs.Front());
                    m_Queues[stealIndex]->Jobs.PopFront();
                    return true;
                }
            }

            return false;
        }
    };
}; // namespace Vortex
