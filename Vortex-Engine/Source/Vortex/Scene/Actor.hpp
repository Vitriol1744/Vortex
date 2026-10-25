/*
 * Created by v1tr10l7 on 11.09.2024.
 * Copyright (c) 2024-2024, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include <Vortex/Core/Core.hpp>
#include <Vortex/Scene/Scene.hpp>

namespace Vortex
{
    using Entity     = entt::entity;
    using EntityNull = entt::null_t;

    class Actor
    {
      public:
        Actor() = default;
        Actor(Entity id, Scene& scene)
            : m_ID(id)
            , m_Scene(&scene)
        {
        }
        ~Actor() = default;

        inline Actor& operator=(entt::null_t)
        {
            m_ID    = entt::null;
            m_Scene = nullptr;
            return *this;
        }

        inline     operator Entity() const { return m_ID; }
        inline     operator bool() const { return m_ID != entt::null; }
        StringView Name();

        template <typename T>
        T& GetComponent()
        {
            VtCoreAssertFormat(HasComponent<T>(),
                               "The entity doesn't have the component '{}'",
                               VtGetTypeName(T));

            return m_Scene->Registry().get<T>(m_ID);
        }

        template <typename T, typename... Args>
        T& AddComponent(Args&&... args)
        {
            VtCoreAssertFormat(!HasComponent<T>(),
                               "The entity already has a component '{}'",
                               VtGetTypeName(T));

            return m_Scene->Registry().emplace<T>(m_ID, Forward<Args>(args)...);
        }
        template <typename T>
        void RemoveComponent()
        {
            VtCoreAssertFormat(HasComponent<T>(),
                               "The entity doesn't has component '{}'",
                               VtGetTypeName(T));

            m_Scene->Registry().remove<T>(m_ID);
        }

        template <typename T>
        inline bool HasComponent()
        {
            return m_Scene->Registry().any_of<T>(m_ID);
        }

      private:
        Entity m_ID    = entt::null;
        Scene* m_Scene = nullptr;
    };
}; // namespace Vortex
