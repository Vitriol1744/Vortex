/*
 * Created by v1tr10l7 on 11.09.2024.
 * Copyright (c) 2024-2024, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#include "vtpch.hpp"

#include "Vortex/Renderer/Renderer2D.hpp"
#include "Vortex/Scene/Entity.hpp"
#include "Vortex/Scene/Scene.hpp"

namespace Vortex
{
    Entity Scene::AddEntity()
    {
        Entity entity = Entity(m_Registry.create(), *this);

        return entity;
    }

    void Scene::DrawEntities()
    {
        auto group
            = m_Registry.group<TransformComponent>(entt::get<SpriteComponent>);
        for (auto entity : group)
        {
            auto [transform, sprite]
                = group.get<TransformComponent, SpriteComponent>(entity);

            Renderer2D::DrawQuad(transform, sprite.Color);
        }
    }
} // namespace Vortex
