/*
 * Created by v1tr10l7 on 11.09.2024.
 * Copyright (c) 2024-2024, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include "Vortex/Core/Core.hpp"
#include "Vortex/Scene/Scene.hpp"

namespace Vortex
{
    using EntityID   = entt::entity;
    using EntityNull = entt::null_t;

    class Entity
    {
      public:
        Entity() = default;
        Entity(EntityID id, Scene& scene)
            : m_ID(id)
            , m_Scene(&scene)
        {
        }
        ~Entity() = default;

        inline Entity& operator=(entt::null_t)
        {
            m_ID    = entt::null;
            m_Scene = nullptr;
            return *this;
        }

        inline operator EntityID() const { return m_ID; }
        inline operator bool() const { return m_ID != entt::null; }

        template <typename T>
        T& GetComponent()
        {
            return m_Scene->GetRegistry().get<T>(m_ID);
        }

        template <typename T, typename... Args>
        T& AddComponent(Args&&... args)
        {
            VtCoreAssertFormat(!HasComponent<T>(),
                               "The entity already has a component '{}'",
                               typeid(T).name());

            return m_Scene->GetRegistry().emplace<T>(
                m_ID, std::forward<Args>(args)...);
        }
        template <typename T>
        void RemoveComponent()
        {
            VtCoreAssertFormat(HasComponent<T>(),
                               "The entity doesn't has component '{}'",
                               typeid(T).name());

            m_Scene->GetRegistry().remove<T>(m_ID);
        }

        template <typename T>
        inline bool HasComponent()
        {
            return m_Scene->GetRegistry().any_of<T>(m_ID);
        }

      private:
        EntityID m_ID    = entt::null;
        Scene*   m_Scene = nullptr;
    };
}; // namespace Vortex
