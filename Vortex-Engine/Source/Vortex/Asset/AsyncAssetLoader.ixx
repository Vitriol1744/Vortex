/*
 * Created by v1tr10l7 on 18.11.2026.
 * Copyright (c) 2024-2026, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
module;

#include <Vortex/Asset/Export.hpp>
#include <Vortex/Core/Types.hpp>

#include <atomic>
#include <condition_variable>
#include <deque>
#include <functional>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <unordered_map>
#include <utility>
#include <vector>

export module Vortex.Asset.AsyncLoader;

import Prism.Memory.Ref;
import Prism.Utility.Path;
import Prism.Utility.PathView;

/**
 * Loads assets on worker threads.
 *
 *   auto mesh = loader.Load<Mesh>("assets/models/cube.obj",
 *                                 [](PathView path) { return CreateRef<Mesh>(path); });
 *   mesh.Then([](const Ref<Mesh>& loaded) { ... });   // main thread, once ready
 *   ...
 *   loader.Update();                                   // main thread, every frame
 *
 * The load function runs on a worker, so it must only do thread-safe work
 * (file IO, decoding, creating RHI resources: the device allows that from any
 * thread). Requests are cached by path: loading the same path again returns
 * the same request, finished or not. Completion callbacks (Then) run on the
 * thread calling Update(), never on a worker.
 */
export extern "C++"
{
    namespace Vortex
    {
        enum class AssetLoadState : u8
        {
            eQueued,
            eLoading,
            eReady,
            eFailed,
        };

        /// The type-erased part of one load: what the workers run.
        class VT_ASSET_API AssetRequestBase
        {
          public:
            explicit AssetRequestBase(PathView path, const void* type)
                : m_Path(path)
                , m_Type(type)
            {
            }
            virtual ~AssetRequestBase() = default;

            AssetRequestBase(const AssetRequestBase&)            = delete;
            AssetRequestBase& operator=(const AssetRequestBase&) = delete;

            inline AssetLoadState State() const
            {
                return m_State.load(std::memory_order_acquire);
            }
            inline bool     IsDone() const { return State() >= AssetLoadState::eReady; }
            inline PathView FilePath() const { return m_Path; }
            /// Identifies the asset type the request was made for.
            inline const void* Type() const { return m_Type; }

          protected:
            friend class AsyncAssetLoader;

            /// Worker thread: loads, then publishes the state.
            virtual void Execute()  = 0;
            /// Update() thread: runs the completion callbacks.
            virtual void Complete() = 0;

            std::atomic<AssetLoadState> m_State{AssetLoadState::eQueued};
            Path                        m_Path;
            const void*                 m_Type = nullptr;
        };

        template <typename T>
        class AssetRequest final : public AssetRequestBase
        {
          public:
            using LoadFunction     = std::function<::Ref<T>(PathView)>;
            using CompleteFunction = std::function<void(const ::Ref<T>&)>;

            /// One address per asset type, to tell cached requests apart.
            static const void* TypeKey()
            {
                static const char key = 0;
                return &key;
            }

            AssetRequest(PathView path, LoadFunction load)
                : AssetRequestBase(path, TypeKey())
                , m_Load(std::move(load))
            {
            }

            /// The asset once State() is eReady, null before (and on failure).
            inline ::Ref<T> Get() const
            {
                return State() == AssetLoadState::eReady ? m_Value : ::Ref<T>(nullptr);
            }

            /// Update() thread only. Runs right away if the load is done.
            void Then(CompleteFunction callback)
            {
                if (IsDone()) callback(Get());
                else m_Callbacks.push_back(std::move(callback));
            }

          protected:
            void Execute() override
            {
                m_State.store(AssetLoadState::eLoading, std::memory_order_release);
                ::Ref<T> value = m_Load ? m_Load(m_Path) : ::Ref<T>(nullptr);
                m_Load         = nullptr;
                m_Value        = value;
                m_State.store(value ? AssetLoadState::eReady : AssetLoadState::eFailed,
                              std::memory_order_release);
            }
            void Complete() override
            {
                auto callbacks = std::move(m_Callbacks);
                m_Callbacks.clear();
                for (auto& callback : callbacks) callback(Get());
            }

          private:
            LoadFunction                  m_Load;
            ::Ref<T>                      m_Value = nullptr;
            std::vector<CompleteFunction> m_Callbacks;
        };

        /// A handle to a (possibly unfinished) load. Cheap to copy.
        template <typename T>
        class AsyncAsset
        {
          public:
            AsyncAsset() = default;
            explicit AsyncAsset(std::shared_ptr<AssetRequest<T>> request)
                : m_Request(std::move(request))
            {
            }

            inline bool           Valid() const { return m_Request != nullptr; }
            inline AssetLoadState State() const
            {
                return m_Request ? m_Request->State() : AssetLoadState::eFailed;
            }
            inline bool     IsReady() const { return State() == AssetLoadState::eReady; }
            inline bool     IsDone() const { return !m_Request || m_Request->IsDone(); }
            inline ::Ref<T> Get() const
            {
                return m_Request ? m_Request->Get() : ::Ref<T>(nullptr);
            }

            /// `callback(asset)` on the Update() thread once loaded (asset is
            /// null if the load failed). Call it from that thread.
            const AsyncAsset& Then(typename AssetRequest<T>::CompleteFunction callback) const
            {
                if (m_Request) m_Request->Then(std::move(callback));
                else callback(nullptr);
                return *this;
            }

          private:
            std::shared_ptr<AssetRequest<T>> m_Request;
        };

        class VT_ASSET_API AsyncAssetLoader
        {
          public:
            explicit AsyncAssetLoader(u32 threadCount = 2);
            /// Finishes the loads already running, drops the queued ones.
            ~AsyncAssetLoader();

            AsyncAssetLoader(const AsyncAssetLoader&)            = delete;
            AsyncAssetLoader& operator=(const AsyncAssetLoader&) = delete;

            /// Queues `load(path)` on a worker, or returns the request already
            /// made for `path`.
            template <typename T, typename F>
            AsyncAsset<T> Load(PathView path, F&& load)
            {
                std::string key(path.Raw(), path.Size());
                {
                    std::lock_guard lock(m_CacheMutex);
                    if (auto it = m_Cache.find(key); it != m_Cache.end())
                    {
                        if (it->second->Type() == AssetRequest<T>::TypeKey())
                            return AsyncAsset<T>(
                                std::static_pointer_cast<AssetRequest<T>>(it->second));
                    }
                }

                auto request = std::make_shared<AssetRequest<T>>(
                    path, typename AssetRequest<T>::LoadFunction(std::forward<F>(load)));
                {
                    std::lock_guard lock(m_CacheMutex);
                    m_Cache[key] = request;
                }
                Enqueue(request);
                return AsyncAsset<T>(std::move(request));
            }

            /// Runs the callbacks of finished loads. Once a frame, on the
            /// thread that owns the assets (the main thread).
            void  Update();
            /// Blocks until nothing is queued or loading, then Update()s.
            void  WaitIdle();

            /// Loads queued or running.
            usize Pending() const;
            /// Forgets finished requests (assets stay alive while used).
            void  ClearCache();

          private:
            using RequestPtr = std::shared_ptr<AssetRequestBase>;

            std::vector<std::thread>                    m_Workers;
            std::deque<RequestPtr>                      m_Queue;
            std::vector<RequestPtr>                     m_Completed;
            std::unordered_map<std::string, RequestPtr> m_Cache;
            mutable std::mutex                          m_QueueMutex;
            std::mutex                                  m_CompletedMutex;
            std::mutex                                  m_CacheMutex;
            std::condition_variable                     m_Wake;
            std::condition_variable                     m_Idle;
            usize                                       m_Running  = 0;
            bool                                        m_Stopping = false;

            void Enqueue(RequestPtr request);
            void WorkerLoop();
        };
    } // namespace Vortex
}
