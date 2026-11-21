/*
 * Created by v1tr10l7 on 18.11.2026.
 * Copyright (c) 2024-2026, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#include <Vortex/Core/Profiler.hpp>
#include <Vortex/Core/Types.hpp>

#include <condition_variable>
#include <deque>
#include <memory>
#include <mutex>
#include <thread>
#include <utility>
#include <vector>

import Vortex.Asset.AsyncLoader;

namespace Vortex
{
    AsyncAssetLoader::AsyncAssetLoader(u32 threadCount)
    {
        if (threadCount == 0) threadCount = 1;
        m_Workers.reserve(threadCount);
        for (u32 i = 0; i < threadCount; ++i)
            m_Workers.emplace_back([this] { WorkerLoop(); });
    }

    AsyncAssetLoader::~AsyncAssetLoader()
    {
        {
            std::lock_guard lock(m_QueueMutex);
            m_Stopping = true;
            m_Queue.clear();
        }
        m_Wake.notify_all();
        for (auto& worker : m_Workers)
            if (worker.joinable()) worker.join();
    }

    void AsyncAssetLoader::Enqueue(RequestPtr request)
    {
        {
            std::lock_guard lock(m_QueueMutex);
            m_Queue.push_back(std::move(request));
        }
        m_Wake.notify_one();
    }

    void AsyncAssetLoader::WorkerLoop()
    {
        VtProfilerSetThreadName("Asset Loader");
        for (;;)
        {
            RequestPtr request;
            {
                std::unique_lock lock(m_QueueMutex);
                m_Wake.wait(lock, [this] { return m_Stopping || !m_Queue.empty(); });
                if (m_Stopping) return;
                request = std::move(m_Queue.front());
                m_Queue.pop_front();
                ++m_Running;
            }

            {
                VtProfileScope("Load asset");
                request->Execute();
            }

            {
                std::lock_guard lock(m_CompletedMutex);
                m_Completed.push_back(request);
            }
            {
                std::lock_guard lock(m_QueueMutex);
                --m_Running;
            }
            m_Idle.notify_all();
        }
    }

    void AsyncAssetLoader::Update()
    {
        VtProfileFunction();
        std::vector<RequestPtr> completed;
        {
            std::lock_guard lock(m_CompletedMutex);
            completed.swap(m_Completed);
        }
        for (auto& request : completed) request->Complete();
    }

    void AsyncAssetLoader::WaitIdle()
    {
        {
            std::unique_lock lock(m_QueueMutex);
            m_Idle.wait(lock, [this] { return m_Queue.empty() && m_Running == 0; });
        }
        Update();
    }

    usize AsyncAssetLoader::Pending() const
    {
        std::lock_guard lock(m_QueueMutex);
        return m_Queue.size() + m_Running;
    }

    void AsyncAssetLoader::ClearCache()
    {
        std::lock_guard lock(m_CacheMutex);
        for (auto it = m_Cache.begin(); it != m_Cache.end();)
        {
            if (it->second->IsDone()) it = m_Cache.erase(it);
            else ++it;
        }
    }
} // namespace Vortex
