/*
 * Created by v1tr10l7 on 28.07.2024.
 * Copyright (c) 2024-2024, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include <Prism/Memory/RefCounted.hpp>
#include <Prism/String/String.hpp>
#include <Vortex/Core/Export.hpp>
#include <Vortex/Core/Timestep.hpp>

namespace Vortex
{
    class VT_API Layer : public RefCounted
    {
      public:
        Layer(StringView name = "Layer")
            : m_Name(name)
        {
        }
        virtual ~Layer() = default;

        virtual void      OnAttach() {}
        virtual void      OnDetach() {}

        virtual void      OnUpdate(Timestep) {}
        virtual void      OnRender() {}
        virtual void      OnImGuiRender() {}

        inline StringView Name() const { return m_Name; }

      private:
        String m_Name = "Layer";
    };
}; // namespace Vortex
